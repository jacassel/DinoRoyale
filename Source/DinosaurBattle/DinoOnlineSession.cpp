#include "DinoOnlineSession.h"
#include "DinoGameMode.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "EOSSettings.h"
#include "OnlineSubsystemEOSTypesPublic.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/NetworkVersion.h"
#include "OnlineSubsystemUtils.h"
#include "DinoOnlineDiagnostics.h"
#include "Engine/PendingNetGame.h"

DEFINE_LOG_CATEGORY_STATIC(LogDinoOnline, Log, All);

namespace
{
const FName ModeKey(TEXT("DINO_MODE")),BotsKey(TEXT("DINO_BOTS")),NameKey(TEXT("DINO_NAME"));
bool CheckCompatibility(const FOnlineSessionSettings& Settings,const TCHAR* Path)
{
    const auto Remote=DinoCompatibility::Read(Settings);
    UE_LOG(LogDinoOnline,Display,TEXT("Compatibility path=%s local=%d remote=%lld source=%s remoteBuildUniqueId=%d decision=%s reason=%s"),
        Path,DinoCompatibility::Build,Remote.Value,Remote.Source,Settings.BuildUniqueId,
        Remote.IsCompatible()?TEXT("ACCEPT"):TEXT("REJECT"),Remote.IsCompatible()?TEXT("equal"):Remote.bValid?TEXT("different build"):TEXT("missing or invalid metadata"));
    return Remote.IsCompatible();
}
}
void UDinoOnlineSession::Initialize(FSubsystemCollectionBase& C)
{
    Super::Initialize(C);
    FString Name; if(GConfig->GetString(TEXT("Dino.Online"),TEXT("Provider"),Name,GGameIni)&&!Name.IsEmpty())Provider=FName(*Name);
    for(const auto& Def:GEngine->NetDriverDefinitions)if(Def.DefName==NAME_GameNetDriver)
        UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] config provider=%s driver=%s fallback=%s"),*Provider.ToString(),*Def.DriverClassName.ToString(),*Def.DriverClassNameFallback.ToString());
    FString Relay;GConfig->GetString(TEXT("SocketSubsystemEOS"),TEXT("RelayControl"),Relay,GEngineIni);
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] config forceRelays=%d StartSessionUsed=0 lobbySession=1"),Relay==TEXT("ForceRelays"));
    UE_LOG(LogDinoOnline,Display,TEXT("Local compatibility ID=%d source=DinoCompatibility::Build OSSBuildUniqueId=%d networkChecksum=%u engineCompatibleChangelist=%u projectVersion=%s"),
        BuildVersion,GetBuildUniqueId(),FNetworkVersion::GetLocalNetworkVersion(),FNetworkVersion::GetNetworkCompatibleChangelist(),*FNetworkVersion::GetProjectVersion());
    NetworkHandle=GEngine->OnNetworkFailure().AddUObject(this,&UDinoOnlineSession::OnNetworkFailure);
    TravelHandle=GEngine->OnTravelFailure().AddUObject(this,&UDinoOnlineSession::OnTravelFailure);
    TickHandle=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&UDinoOnlineSession::Tick));
}
void UDinoOnlineSession::Deinitialize()
{
    FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
    if(GEngine){GEngine->OnNetworkFailure().Remove(NetworkHandle);GEngine->OnTravelFailure().Remove(TravelHandle);}
    if(Identity)Identity->ClearOnLoginCompleteDelegate_Handle(0,LoginHandle);
    if(Sessions)
    {
        Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
        Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
        Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
        Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
        Sessions->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteHandle);
        if(Sessions->GetNamedSession(NAME_GameSession))Sessions->DestroySession(NAME_GameSession);
    }
    Super::Deinitialize();
}
bool UDinoOnlineSession::EnsureProvider()
{
    if(bTimedOut){Status=TEXT("Online request timed out. Restart the game to retry; offline play is available.");return false;}
    if(Sessions&&Identity)return true;
    if(Provider==TEXT("NULL")||Provider==NAME_None){Status=TEXT("Internet play requires a configured online provider.");return false;}
    if(Provider==TEXT("EOS"))
    {
        // A loose per-product configuration allows the packaged build to be
        // configured without Unreal, a compiler, or putting credentials in Git.
        FConfigFile File;File.Read(FPaths::ProjectDir()/TEXT("OnlineServices.ini"));
        FArtifactSettings A;A.ArtifactName=TEXT("DinosaurBattle");
        File.GetString(TEXT("Dino.EOS"),TEXT("ProductId"),A.ProductId);
        File.GetString(TEXT("Dino.EOS"),TEXT("SandboxId"),A.SandboxId);
        File.GetString(TEXT("Dino.EOS"),TEXT("DeploymentId"),A.DeploymentId);
        File.GetString(TEXT("Dino.EOS"),TEXT("ClientId"),A.ClientId);
        File.GetString(TEXT("Dino.EOS"),TEXT("ClientSecret"),A.ClientSecret);
        if(A.ProductId.IsEmpty()||A.SandboxId.IsEmpty()||A.DeploymentId.IsEmpty()||A.ClientId.IsEmpty()||A.ClientSecret.IsEmpty()||A.ProductId.Contains(TEXT("REPLACE")))
        {Status=TEXT("EOS needs product configuration. See EOS_SETUP.md and OnlineServices.example.ini.");return false;}
        A.ClientEncryptionKey=FString::ChrN(64,TCHAR('1')); // No title/player storage is used.
        auto* Settings=GetMutableDefault<UEOSSettings>();
        Settings->DefaultArtifactName=A.ArtifactName;Settings->Artifacts={A};Settings->bUseEAS=true;Settings->bUseEOSConnect=true;
        Settings->bEnableOverlay=true;Settings->bEnableSocialOverlay=true;Settings->bEnableEditorOverlay=true;
    }
    Online=IOnlineSubsystem::Get(Provider);
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] initialize provider=%s initialized=%d actualSubsystem=%s"),*Provider.ToString(),Online!=nullptr,Online?*Online->GetSubsystemName().ToString():TEXT("none"));
    if(!Online){Status=TEXT("Online service could not initialize. Check product configuration and restart.");return false;}
    Sessions=Online->GetSessionInterface();Identity=Online->GetIdentityInterface();
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] interfaces sessions=%d identity=%d lobbyViaSessionInterface=1 externalUI=%d"),Sessions.IsValid(),Identity.IsValid(),Online->GetExternalUIInterface().IsValid());
    if(!Sessions||!Identity){Status=TEXT("Online sessions or sign-in are unavailable.");Sessions.Reset();Identity.Reset();return false;}
    LoginHandle=Identity->AddOnLoginCompleteDelegate_Handle(0,FOnLoginCompleteDelegate::CreateUObject(this,&UDinoOnlineSession::OnLogin));
    CreateHandle=Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this,&UDinoOnlineSession::OnCreate));
    FindHandle=Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this,&UDinoOnlineSession::OnFind));
    JoinHandle=Sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this,&UDinoOnlineSession::OnJoin));
    DestroyHandle=Sessions->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this,&UDinoOnlineSession::OnDestroy));
    InviteHandle=Sessions->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this,&UDinoOnlineSession::OnInvite));
    return true;
}
bool UDinoOnlineSession::IsSignedIn() const{return Identity&&Identity->GetLoginStatus(0)==ELoginStatus::LoggedIn;}
bool UDinoOnlineSession::ValidateIdentity(const FUniqueNetIdRepl& ID) const
{return Identity&&ID.IsValid()&&ID.GetType()==Provider;}
FString UDinoOnlineSession::Nickname() const{return IsSignedIn()?Identity->GetPlayerNickname(0):FString();}
void UDinoOnlineSession::EnterNetworkWorld()
{
    DinoOnlineDiagnostics::World(GetWorld(),TEXT("network-world-entered"));
    DinoOnlineDiagnostics::NamedSession(Sessions,TEXT("network-world-entered"));
    bInSession=true;bConnected=true;bHosting=GetWorld()->GetNetMode()==NM_ListenServer;Status=bHosting?TEXT("Hosting online match."):TEXT("Connected to host.");
    const auto* Driver=GetWorld()->GetNetDriver();
    UE_LOG(LogDinoOnline,Display,TEXT("Network world entered hosting=%d netDriver=%s compatibility=%d"),bHosting,Driver?*Driver->GetClass()->GetName():TEXT("none"),BuildVersion);
}
void UDinoOnlineSession::BeginOperation(EOperation Op,const FString& Message)
{Operation=Op;bBusy=true;Deadline=FPlatformTime::Seconds()+(Op==EOperation::Login?180:45);Status=Message;UE_LOG(LogDinoOnline,Display,TEXT("Operation started kind=%d timeoutSeconds=%d"),int32(Op),Op==EOperation::Login?180:45);}
bool UDinoOnlineSession::Tick(float)
{
    if(bInSession&&!bHosting&&!bConnected&&GEngine)
        if(const auto* Context=GEngine->GetWorldContextFromWorld(GetWorld()))
            if(Context->PendingNetGame&&Context->PendingNetGame->NetDriver&&LastPendingDriver.Get()!=Context->PendingNetGame->NetDriver)
            {
                LastPendingDriver=Context->PendingNetGame->NetDriver;
                DinoOnlineDiagnostics::World(GetWorld(),TEXT("ClientTravel-pending-driver"),Context->PendingNetGame->NetDriver);
            }
    if(bBusy&&FPlatformTime::Seconds()>Deadline)
    {
        UE_LOG(LogDinoOnline,Warning,TEXT("Operation timed out kind=%d searchState=%d results=%d; restart required to discard pending provider callbacks"),int32(Operation),CurrentSearch?int32(CurrentSearch->SearchState):-1,CurrentSearch?CurrentSearch->SearchResults.Num():0);
        bBusy=false;bTimedOut=true;bLeaving=true;Operation=EOperation::None;
        Status=TEXT("Online request timed out. Offline play is available; restart to retry online.");
        if(Sessions&&Sessions->GetNamedSession(NAME_GameSession))Sessions->DestroySession(NAME_GameSession);
        if(bInSession)ReturnToMenu();
    }
    return true;
}
void UDinoOnlineSession::SignIn()
{
    if(bBusy||!EnsureProvider())return;
    if(IsSignedIn()){if(auto* Player=GetGameInstance()->GetLocalPlayerByIndex(0))Player->SetCachedUniqueNetId(FUniqueNetIdRepl(Identity->GetUniquePlayerId(0)));Status=TEXT("Signed in as ")+Identity->GetPlayerNickname(0);return;}
    bLeaving=false;BeginOperation(EOperation::Login,TEXT("Complete Epic sign-in in the browser window."));
    const bool Started=Provider==TEXT("EOS")?Identity->Login(0,FOnlineAccountCredentials(TEXT("accountportal"),TEXT(""),TEXT(""))):Identity->AutoLogin(0);
    if(!Started){bBusy=false;Operation=EOperation::None;Status=TEXT("Sign-in could not start. Check the online configuration.");}
}
void UDinoOnlineSession::OnLogin(int32,bool Success,const FUniqueNetId&,const FString&)
{
    DinoOnlineDiagnostics::Identity(Identity,TEXT("Login-complete"));
    UE_LOG(LogDinoOnline,Display,TEXT("Login callback success=%d"),Success);
    if(Operation!=EOperation::Login)return;bBusy=false;Operation=EOperation::None;
    if(Success)if(auto* Player=GetGameInstance()->GetLocalPlayerByIndex(0))Player->SetCachedUniqueNetId(FUniqueNetIdRepl(Identity->GetUniquePlayerId(0)));
    Status=Success?TEXT("Signed in as ")+Identity->GetPlayerNickname(0):TEXT("Epic sign-in failed or was cancelled. Check access to this EOS product.");
}
void UDinoOnlineSession::Host(bool Teams,int32 Capacity,bool Bots,bool Public,bool PerformanceMap)
{
    if(bBusy||!EnsureProvider())return;
    if(!IsSignedIn()){Status=TEXT("Sign in before hosting.");return;}
    if(Sessions->GetNamedSession(NAME_GameSession)){Status=TEXT("Leave the current session before hosting another.");return;}
    Capacity=FMath::Clamp(Capacity,2,10);bPublic=Public;bLeaving=false;
    FOnlineSessionSettings S;S.bIsLANMatch=false;S.bShouldAdvertise=Public;S.bUsesPresence=true;S.bUseLobbiesIfAvailable=true;
    S.bAllowJoinInProgress=true;S.bAllowInvites=true;S.bAllowJoinViaPresence=Public;S.bAllowJoinViaPresenceFriendsOnly=false;
    S.NumPublicConnections=Public?Capacity:0;S.NumPrivateConnections=Public?0:Capacity;S.BuildUniqueId=BuildVersion;
    S.Set(SETTING_HOST_MIGRATION,false,EOnlineDataAdvertisementType::DontAdvertise);
    if(Provider==TEXT("EOS"))S.Set(OSSEOS_BUCKET_ID_ATTRIBUTE_KEY,DinoCompatibility::Bucket(),EOnlineDataAdvertisementType::ViaOnlineService);
    DinoCompatibility::Advertise(S);
    CheckCompatibility(S,TEXT("host advertise"));
    UE_LOG(LogDinoOnline,Display,TEXT("Host create bucket=%s capacity=%d public=%d lobbies=%d presence=%d"),*DinoCompatibility::Bucket(),Capacity,Public,S.bUseLobbiesIfAvailable,S.bUsesPresence);
    S.Set(ModeKey,Teams?FString(TEXT("Team Battle")):FString(TEXT("Free-for-All")),EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(BotsKey,Bots,EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(NameKey,Identity->GetPlayerNickname(0).Left(48)+TEXT("'s match"),EOnlineDataAdvertisementType::ViaOnlineService);
    PendingOptions=FString::Printf(TEXT("listen?OnlineLobby=1?Capacity=%d?Teams=%d?Bots=%d?DinoBuild=%d"),Capacity,Teams?1:0,Bots?1:0,BuildVersion);
    PendingOptions+=FString::Printf(TEXT("?PerformanceMap=%d"),PerformanceMap?1:0);
    if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())PendingOptions+=FString::Printf(TEXT("?TeamGoal=%d?AllowedSpecies=%d?ExplicitBots=%d?TeamABots=%d?TeamBBots=%d"),GM->TeamKillGoal,GM->MatchRules.AllowedSpeciesMask,GM->MatchRules.bExplicitBotCounts?1:0,GM->MatchRules.TeamABots,GM->MatchRules.TeamBBots);
    BeginOperation(EOperation::Create,TEXT("Creating EOS lobby..."));
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] CreateSession request name=GameSession lan=%d presence=%d lobby=%d advertise=%d public=%d private=%d joinInProgress=%d invites=%d listenOption=1"),S.bIsLANMatch,S.bUsesPresence,S.bUseLobbiesIfAvailable,S.bShouldAdvertise,S.NumPublicConnections,S.NumPrivateConnections,S.bAllowJoinInProgress,S.bAllowInvites);
    DinoOnlineDiagnostics::Identity(Identity,TEXT("CreateSession-request"));
    if(!Sessions->CreateSession(0,NAME_GameSession,S)){bBusy=false;Operation=EOperation::None;Status=TEXT("Could not create an online lobby.");}
}
void UDinoOnlineSession::OnCreate(FName,bool Success)
{
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] CreateSession complete success=%d advertisementComplete=%d"),Success,Success);
    DinoOnlineDiagnostics::NamedSession(Sessions,TEXT("CreateSession-complete"));
    UE_LOG(LogDinoOnline,Display,TEXT("Create callback success=%d"),Success);
    if(Success&&Sessions)if(const auto* S=Sessions->GetSessionSettings(NAME_GameSession))CheckCompatibility(*S,TEXT("host provider settings"));
    if(Success&&Sessions)
    {
        FString ConnectString;const bool Resolved=Sessions->GetResolvedConnectString(NAME_GameSession,ConnectString,NAME_GamePort);
        DinoOnlineDiagnostics::Resolution(Resolved,ConnectString,TEXT("host-created"));
        UE_LOG(LogDinoOnline,Display,TEXT("Host connect string resolved=%d eosP2P=%d (address redacted)"),Resolved,DinoEOSAddress::IsValid(ConnectString));
    }
    if(bLeaving||Operation!=EOperation::Create){if(Success){BeginOperation(EOperation::Destroy,Status);Sessions->DestroySession(NAME_GameSession);}else{bBusy=false;Operation=EOperation::None;}return;}
    bBusy=false;Operation=EOperation::None;
    if(!Success){Status=TEXT("Lobby creation failed. Check internet access and EOS client policy.");return;}
    bInSession=true;bHosting=true;Status=TEXT("Lobby created. Waiting for players.");
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] OpenLevel request map=/Game/Maps/LostValley listen=1"));
    UGameplayStatics::OpenLevel(GetGameInstance(),TEXT("/Game/Maps/LostValley"),true,PendingOptions);
}
void UDinoOnlineSession::Search()
{
    if(bBusy||!EnsureProvider())return;if(!IsSignedIn()){Status=TEXT("Sign in before searching.");return;}
    Results.Reset();CurrentSearch=MakeShared<FOnlineSessionSearch>();CurrentSearch->bIsLanQuery=false;CurrentSearch->MaxSearchResults=50;
    CurrentSearch->QuerySettings.Set(SEARCH_LOBBIES,true,EOnlineComparisonOp::Equals);
    CurrentSearch->QuerySettings.Set(DinoCompatibility::Key,BuildVersion,EOnlineComparisonOp::Equals); // EOS converts this numeric query to Int64.
    if(Provider==TEXT("EOS"))CurrentSearch->QuerySettings.Set(OSSEOS_BUCKET_ID_ATTRIBUTE_KEY,DinoCompatibility::Bucket(),EOnlineComparisonOp::Equals);
    UE_LOG(LogDinoOnline,Display,TEXT("Search requested bucket=%s DINO_BUILD=%d lobbies=1"),*DinoCompatibility::Bucket(),BuildVersion);
    BeginOperation(EOperation::Search,TEXT("Searching internet lobbies..."));
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] FindSessions request lan=%d lobbies=1 maxResults=%d"),CurrentSearch->bIsLanQuery,CurrentSearch->MaxSearchResults);
    if(!Sessions->FindSessions(0,CurrentSearch.ToSharedRef())){bBusy=false;Operation=EOperation::None;Status=TEXT("Session search could not start.");}
}
void UDinoOnlineSession::OnFind(bool Success)
{
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] FindSessions complete success=%d resultCount=%d"),Success,CurrentSearch?CurrentSearch->SearchResults.Num():0);
    UE_LOG(LogDinoOnline,Display,TEXT("Search callback success=%d activeOperation=%d rawResults=%d"),Success,int32(Operation),CurrentSearch?CurrentSearch->SearchResults.Num():0);
    if(Operation!=EOperation::Search)return;bBusy=false;Operation=EOperation::None;Results.Reset();
    if(Success&&CurrentSearch)for(const auto& R:CurrentSearch->SearchResults)
    {
        if(CheckCompatibility(R.Session.SessionSettings,TEXT("browser result"))&&R.IsValid()&&R.Session.NumOpenPublicConnections>0)Results.Add(R);
    }
    Status=!Success?TEXT("Internet session search failed."):Results.IsEmpty()?TEXT("No compatible public sessions found. Try Refresh or an invite."):FString::Printf(TEXT("%d compatible sessions found."),Results.Num());
}
FString UDinoOnlineSession::ResultLabel(int32 Index) const
{
    if(!Results.IsValidIndex(Index))return TEXT("");const auto& R=Results[Index];const auto& S=R.Session.SessionSettings;
    FString Name,Mode;bool Bots=false;S.Get(NameKey,Name);S.Get(ModeKey,Mode);S.Get(BotsKey,Bots);
    const int32 Max=S.NumPublicConnections+S.NumPrivateConnections,Open=R.Session.NumOpenPublicConnections+R.Session.NumOpenPrivateConnections;
    return FString::Printf(TEXT("%s | %s | %d/%d players | Bots %s"),*Name.Left(48),*Mode,FMath::Clamp(Max-Open,0,Max),Max,Bots?TEXT("ON"):TEXT("OFF"));
}
void UDinoOnlineSession::Join(int32 Index)
{if(Results.IsValidIndex(Index))JoinResult(Results[Index]);}
void UDinoOnlineSession::JoinResult(const FOnlineSessionSearchResult& R)
{
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] selectedResult valid=%d"),R.IsValid());
    DinoOnlineDiagnostics::Session(&R.Session,TEXT("selected-result"));
    if(bBusy||!EnsureProvider())return;if(!IsSignedIn()){Status=TEXT("Sign in before joining.");return;}
    if(!CheckCompatibility(R.Session.SessionSettings,TEXT("join gate")))
    {
        Status=DinoCompatibility::Read(R.Session.SessionSettings).bValid?TEXT("Different game version. Both players need the same build."):TEXT("Lobby version information is missing or invalid. Ask the host to recreate it.");
        return;
    }
    if(!R.IsValid()){Status=TEXT("This lobby is no longer available. Refresh or request another invite.");return;}
    if(Sessions->GetNamedSession(NAME_GameSession)){Status=TEXT("Leave your current match before accepting another invite.");return;}
    bLeaving=false;BeginOperation(EOperation::Join,TEXT("Joining online session..."));
    FOnlineSessionSearchResult Desired=R;
    // EOS search results deliberately omit presence; opt into its social overlay.
    Desired.Session.SessionSettings.bUsesPresence=true;
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] JoinSession request name=GameSession"));
    UE_LOG(LogDinoOnline,Display,TEXT("Join submitted lobbies=%d presence=%d localBuild=%d"),Desired.Session.SessionSettings.bUseLobbiesIfAvailable,Desired.Session.SessionSettings.bUsesPresence,BuildVersion);
    if(!Sessions->JoinSession(0,NAME_GameSession,Desired)){bBusy=false;Operation=EOperation::None;Status=TEXT("Could not join this session.");}
}
void UDinoOnlineSession::OnJoin(FName,EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] JoinSession complete result=%s value=%d"),LexToString(Result),int32(Result));
    DinoOnlineDiagnostics::NamedSession(Sessions,TEXT("JoinSession-complete"));
    UE_LOG(LogDinoOnline,Display,TEXT("Join callback result=%d activeOperation=%d"),int32(Result),int32(Operation));
    if(bLeaving||Operation!=EOperation::Join){BeginOperation(EOperation::Destroy,Status);if(!Sessions->DestroySession(NAME_GameSession)){bBusy=false;Operation=EOperation::None;ReturnToMenu();}return;}
    bBusy=false;Operation=EOperation::None;
    if(Result!=EOnJoinSessionCompleteResult::Success)
    {
        Status=Result==EOnJoinSessionCompleteResult::SessionIsFull?TEXT("Lobby is full."):TEXT("Join failed. The session may have ended.");
        if(Sessions->GetNamedSession(NAME_GameSession))Sessions->DestroySession(NAME_GameSession);return;
    }
    FString URL;
    const bool Resolved=Sessions->GetResolvedConnectString(NAME_GameSession,URL,NAME_GamePort);
    DinoOnlineDiagnostics::Resolution(Resolved,URL,TEXT("guest-joined"));
    UE_LOG(LogDinoOnline,Display,TEXT("Connect string resolved=%d eosP2P=%d travelBuild=%d (address redacted)"),Resolved,DinoEOSAddress::IsValid(URL),BuildVersion);
    // Use FURL's host, as NetDriverEOS does. UE 5.8 returns [EOS:PUID], so
    // testing the unparsed string for an EOS: prefix incorrectly rejects it.
    if(!Resolved||URL.IsEmpty()||(Provider==TEXT("EOS")&&!DinoEOSAddress::IsValid(URL)))
    {
        UE_LOG(LogDinoOnline,Warning,TEXT("[DINO_EOS] travel rejected reason=%s"),!Resolved?TEXT("GetResolvedConnectString-failed"):TEXT("invalid-or-non-EOS-travel-URL"));
        Status=TEXT("The session did not provide an EOS P2P address.");Sessions->DestroySession(NAME_GameSession);return;
    }
    auto* PC=GetGameInstance()->GetFirstLocalPlayerController();
    if(!PC){Status=TEXT("Local player is unavailable. Return to the menu and retry.");Sessions->DestroySession(NAME_GameSession);return;}
    bInSession=true;bHosting=false;bConnected=false;Status=TEXT("Connecting to host...");
    LastPendingDriver.Reset();
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] ClientTravel requested controllerValid=%d type=Absolute urlShape=%s compatibility=%d"),GetGameInstance()->GetFirstLocalPlayerController()!=nullptr,DinoEOSAddress::Shape(URL),BuildVersion);
    PC->ClientTravel(URL+FString::Printf(TEXT("?DinoBuild=%d"),BuildVersion),TRAVEL_Absolute);
}
void UDinoOnlineSession::Leave(const FString& Reason)
{
    Status=Reason;bLeaving=true;bShowMenuOnReturn=true;
    if(bBusy&&Operation!=EOperation::Search)return; // In-flight creation/join callbacks perform cleanup.
    if(Sessions&&Sessions->GetNamedSession(NAME_GameSession))
    {BeginOperation(EOperation::Destroy,Reason);if(Sessions->DestroySession(NAME_GameSession))return;}
    bBusy=false;Operation=EOperation::None;ReturnToMenu();
}
void UDinoOnlineSession::OnDestroy(FName,bool Success)
{
    UE_LOG(LogDinoOnline,Display,TEXT("[DINO_EOS] DestroySession complete success=%d"),Success);
    if(Operation==EOperation::Destroy){bBusy=false;Operation=EOperation::None;ReturnToMenu();}
}
void UDinoOnlineSession::ReturnToMenu()
{
    const bool WasNetworked=GetWorld()&&GetWorld()->GetNetMode()!=NM_Standalone;
    bInSession=false;bHosting=false;bConnected=false;
    if(WasNetworked)UGameplayStatics::OpenLevel(GetGameInstance(),TEXT("/Game/Maps/LostValley"));
}
void UDinoOnlineSession::InviteFriends()
{
    if(!Online||!bInSession)return;auto UI=Online->GetExternalUIInterface();
    // OSS EOS ShowInviteUI is an unimplemented stub. ShowFriendsUI invokes the
    // same EOS social panel as Shift+F3, including its existing Invite to game.
    const bool Started=UI&&(Provider==TEXT("EOS")?UI->ShowFriendsUI(0):UI->ShowInviteUI(0,NAME_GameSession));
    UE_LOG(LogDinoOnline,Display,TEXT("Social overlay requested started=%d method=%s"),Started,Provider==TEXT("EOS")?TEXT("ShowFriendsUI"):TEXT("ShowInviteUI"));
    Status=Started?TEXT("Choose a friend in the Epic overlay, then Invite to game. Shift+F3 also opens it."):TEXT("Invites are unavailable. Try Shift+F3 or use a public session.");
}
void UDinoOnlineSession::OnInvite(bool Success,int32 User,FUniqueNetIdPtr,const FOnlineSessionSearchResult& Result)
{
    UE_LOG(LogDinoOnline,Display,TEXT("Invite accepted callback success=%d localUser=%d validResult=%d"),Success,User,Result.IsValid());
    if(Success&&User==0){CheckCompatibility(Result.Session.SessionSettings,TEXT("invite callback"));JoinResult(Result);}
}
void UDinoOnlineSession::UpdateHostedSettings(bool Teams,int32 Capacity,bool Bots)
{
    if(!bHosting||!Sessions)return;auto* Current=Sessions->GetSessionSettings(NAME_GameSession);if(!Current)return;
    FOnlineSessionSettings S=*Current;S.NumPublicConnections=bPublic?Capacity:0;S.NumPrivateConnections=bPublic?0:Capacity;
    S.Set(ModeKey,Teams?FString(TEXT("Team Battle")):FString(TEXT("Free-for-All")),EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(BotsKey,Bots,EOnlineDataAdvertisementType::ViaOnlineService);Sessions->UpdateSession(NAME_GameSession,S,true);
}
void UDinoOnlineSession::OnNetworkFailure(UWorld* World,UNetDriver* Driver,ENetworkFailure::Type Failure,const FString& Error)
{
    UE_LOG(LogDinoOnline,Warning,TEXT("[DINO_EOS] NetworkFailure type=%s detailPresent=%d (engine detail kept in local log)"),ENetworkFailure::ToString(Failure),!Error.IsEmpty());
    DinoOnlineDiagnostics::World(World,TEXT("network-failure"),Driver);
    UE_LOG(LogDinoOnline,Warning,TEXT("Network failure kind=%d netDriver=%s hosting=%d connected=%d"),int32(Failure),Driver?*Driver->GetClass()->GetName():TEXT("none"),bHosting,bConnected);
    if((World&&World->GetGameInstance()!=GetGameInstance())||bLeaving)return;
    // A listen server receives these notifications for individual guest connections.
    // NetConnection cleanup invokes GameMode::Logout and replaces that participant;
    // taking down the host session here would disconnect every healthy guest too.
    if(Driver&&Driver->IsServer()&&(Failure==ENetworkFailure::ConnectionTimeout||Failure==ENetworkFailure::ConnectionLost))return;
    const FString Reason=Error.Contains(TEXT("Different game version"))?TEXT("Different game version."):Error.Contains(TEXT("full"),ESearchCase::IgnoreCase)?TEXT("Lobby is full."):Error.Contains(TEXT("Match is ending"))?TEXT("Match is ending."):bHosting?TEXT("Multiplayer connection failed."):bConnected?TEXT("Host disconnected."):TEXT("Could not connect to host.");
    bBusy=false;Operation=EOperation::None;Leave(Reason);
}
void UDinoOnlineSession::OnTravelFailure(UWorld* World,ETravelFailure::Type Failure,const FString& Error)
{
    UE_LOG(LogDinoOnline,Warning,TEXT("[DINO_EOS] TravelFailure type=%s detailPresent=%d (engine detail kept in local log)"),ETravelFailure::ToString(Failure),!Error.IsEmpty());
    if(World&&World->GetGameInstance()!=GetGameInstance())return;
    if(bInSession){bBusy=false;Operation=EOperation::None;Leave(TEXT("Could not load the multiplayer match."));}
}
