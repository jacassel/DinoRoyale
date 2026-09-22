#include "DinoOnlineSession.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "EOSSettings.h"
#include "OnlineSubsystemEOSTypesPublic.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

namespace
{
const FName VersionKey(TEXT("DINO_BUILD")),ModeKey(TEXT("DINO_MODE")),BotsKey(TEXT("DINO_BOTS")),NameKey(TEXT("DINO_NAME"));
}
void UDinoOnlineSession::Initialize(FSubsystemCollectionBase& C)
{
    Super::Initialize(C);
    FString Name; if(GConfig->GetString(TEXT("Dino.Online"),TEXT("Provider"),Name,GGameIni)&&!Name.IsEmpty())Provider=FName(*Name);
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
    if(!Online){Status=TEXT("Online service could not initialize. Check product configuration and restart.");return false;}
    Sessions=Online->GetSessionInterface();Identity=Online->GetIdentityInterface();
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
{bInSession=true;bConnected=true;bHosting=GetWorld()->GetNetMode()==NM_ListenServer;Status=bHosting?TEXT("Hosting online match."):TEXT("Connected to host.");}
void UDinoOnlineSession::BeginOperation(EOperation Op,const FString& Message)
{Operation=Op;bBusy=true;Deadline=FPlatformTime::Seconds()+(Op==EOperation::Login?180:45);Status=Message;}
bool UDinoOnlineSession::Tick(float)
{
    if(bBusy&&FPlatformTime::Seconds()>Deadline)
    {
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
    if(Operation!=EOperation::Login)return;bBusy=false;Operation=EOperation::None;
    if(Success)if(auto* Player=GetGameInstance()->GetLocalPlayerByIndex(0))Player->SetCachedUniqueNetId(FUniqueNetIdRepl(Identity->GetUniquePlayerId(0)));
    Status=Success?TEXT("Signed in as ")+Identity->GetPlayerNickname(0):TEXT("Epic sign-in failed or was cancelled. Check access to this EOS product.");
}
void UDinoOnlineSession::Host(bool Teams,int32 Capacity,bool Bots,bool Public)
{
    if(bBusy||!EnsureProvider())return;
    if(!IsSignedIn()){Status=TEXT("Sign in before hosting.");return;}
    if(Sessions->GetNamedSession(NAME_GameSession)){Status=TEXT("Leave the current session before hosting another.");return;}
    Capacity=FMath::Clamp(Capacity,2,10);bPublic=Public;bLeaving=false;
    FOnlineSessionSettings S;S.bIsLANMatch=false;S.bShouldAdvertise=Public;S.bUsesPresence=true;S.bUseLobbiesIfAvailable=true;
    S.bAllowJoinInProgress=true;S.bAllowInvites=true;S.bAllowJoinViaPresence=Public;S.bAllowJoinViaPresenceFriendsOnly=false;
    S.NumPublicConnections=Public?Capacity:0;S.NumPrivateConnections=Public?0:Capacity;S.BuildUniqueId=BuildVersion;
    S.Set(SETTING_HOST_MIGRATION,false,EOnlineDataAdvertisementType::DontAdvertise);
    if(Provider==TEXT("EOS"))S.Set(OSSEOS_BUCKET_ID_ATTRIBUTE_KEY,FString::Printf(TEXT("DinosaurBattle-%d"),BuildVersion),EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(VersionKey,BuildVersion,EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(ModeKey,Teams?FString(TEXT("Team Battle")):FString(TEXT("Free-for-All")),EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(BotsKey,Bots,EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(NameKey,Identity->GetPlayerNickname(0).Left(48)+TEXT("'s match"),EOnlineDataAdvertisementType::ViaOnlineService);
    PendingOptions=FString::Printf(TEXT("listen?OnlineLobby=1?Capacity=%d?Teams=%d?Bots=%d?DinoBuild=%d"),Capacity,Teams?1:0,Bots?1:0,BuildVersion);
    BeginOperation(EOperation::Create,TEXT("Creating EOS lobby..."));
    if(!Sessions->CreateSession(0,NAME_GameSession,S)){bBusy=false;Operation=EOperation::None;Status=TEXT("Could not create an online lobby.");}
}
void UDinoOnlineSession::OnCreate(FName,bool Success)
{
    if(bLeaving||Operation!=EOperation::Create){if(Success){BeginOperation(EOperation::Destroy,Status);Sessions->DestroySession(NAME_GameSession);}else{bBusy=false;Operation=EOperation::None;}return;}
    bBusy=false;Operation=EOperation::None;
    if(!Success){Status=TEXT("Lobby creation failed. Check internet access and EOS client policy.");return;}
    bInSession=true;bHosting=true;Status=TEXT("Lobby created. Waiting for players.");
    UGameplayStatics::OpenLevel(GetGameInstance(),TEXT("/Game/Maps/LostValley"),true,PendingOptions);
}
void UDinoOnlineSession::Search()
{
    if(bBusy||!EnsureProvider())return;if(!IsSignedIn()){Status=TEXT("Sign in before searching.");return;}
    Results.Reset();CurrentSearch=MakeShared<FOnlineSessionSearch>();CurrentSearch->bIsLanQuery=false;CurrentSearch->MaxSearchResults=50;
    CurrentSearch->QuerySettings.Set(SEARCH_LOBBIES,true,EOnlineComparisonOp::Equals);
    CurrentSearch->QuerySettings.Set(VersionKey,BuildVersion,EOnlineComparisonOp::Equals);
    if(Provider==TEXT("EOS"))CurrentSearch->QuerySettings.Set(OSSEOS_BUCKET_ID_ATTRIBUTE_KEY,FString::Printf(TEXT("DinosaurBattle-%d"),BuildVersion),EOnlineComparisonOp::Equals);
    BeginOperation(EOperation::Search,TEXT("Searching internet lobbies..."));
    if(!Sessions->FindSessions(0,CurrentSearch.ToSharedRef())){bBusy=false;Operation=EOperation::None;Status=TEXT("Session search could not start.");}
}
void UDinoOnlineSession::OnFind(bool Success)
{
    if(Operation!=EOperation::Search)return;bBusy=false;Operation=EOperation::None;Results.Reset();
    if(Success&&CurrentSearch)for(const auto& R:CurrentSearch->SearchResults)
    {
        int32 Version=0;R.Session.SessionSettings.Get(VersionKey,Version);
        if(Version==BuildVersion&&R.IsValid()&&R.Session.NumOpenPublicConnections>0)Results.Add(R);
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
    if(bBusy||!EnsureProvider())return;if(!IsSignedIn()){Status=TEXT("Sign in before joining.");return;}
    int32 Version=0;R.Session.SessionSettings.Get(VersionKey,Version);
    if(Version!=BuildVersion){Status=TEXT("Different game version. Both players need the same build.");return;}
    if(Sessions->GetNamedSession(NAME_GameSession)){Status=TEXT("Leave your current match before accepting another invite.");return;}
    bLeaving=false;BeginOperation(EOperation::Join,TEXT("Joining online session..."));
    FOnlineSessionSearchResult Desired=R;
    // EOS search results deliberately omit presence; opt into its social overlay.
    Desired.Session.SessionSettings.bUsesPresence=true;
    if(!Sessions->JoinSession(0,NAME_GameSession,Desired)){bBusy=false;Operation=EOperation::None;Status=TEXT("Could not join this session.");}
}
void UDinoOnlineSession::OnJoin(FName,EOnJoinSessionCompleteResult::Type Result)
{
    if(bLeaving||Operation!=EOperation::Join){BeginOperation(EOperation::Destroy,Status);if(!Sessions->DestroySession(NAME_GameSession)){bBusy=false;Operation=EOperation::None;ReturnToMenu();}return;}
    bBusy=false;Operation=EOperation::None;
    if(Result!=EOnJoinSessionCompleteResult::Success)
    {
        Status=Result==EOnJoinSessionCompleteResult::SessionIsFull?TEXT("Lobby is full."):TEXT("Join failed. The session may have ended.");
        if(Sessions->GetNamedSession(NAME_GameSession))Sessions->DestroySession(NAME_GameSession);return;
    }
    FString URL;
    if(!Sessions->GetResolvedConnectString(NAME_GameSession,URL)||(Provider==TEXT("EOS")&&!URL.StartsWith(TEXT("EOS:"),ESearchCase::IgnoreCase)))
    {Status=TEXT("The session did not provide an EOS P2P address.");Sessions->DestroySession(NAME_GameSession);return;}
    bInSession=true;bHosting=false;bConnected=false;Status=TEXT("Connecting to host...");
    if(auto* PC=GetGameInstance()->GetFirstLocalPlayerController())PC->ClientTravel(URL+FString::Printf(TEXT("?DinoBuild=%d"),BuildVersion),TRAVEL_Absolute);
}
void UDinoOnlineSession::Leave(const FString& Reason)
{
    Status=Reason;bLeaving=true;bShowMenuOnReturn=true;
    if(bBusy&&Operation!=EOperation::Search)return; // In-flight creation/join callbacks perform cleanup.
    if(Sessions&&Sessions->GetNamedSession(NAME_GameSession))
    {BeginOperation(EOperation::Destroy,Reason);if(Sessions->DestroySession(NAME_GameSession))return;}
    bBusy=false;Operation=EOperation::None;ReturnToMenu();
}
void UDinoOnlineSession::OnDestroy(FName,bool)
{
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
    if(!UI||!UI->ShowInviteUI(0,NAME_GameSession))Status=TEXT("Invites are unavailable. Check the Epic overlay or use a public session.");
}
void UDinoOnlineSession::OnInvite(bool Success,int32 User,FUniqueNetIdPtr,const FOnlineSessionSearchResult& Result)
{if(Success&&User==0)JoinResult(Result);}
void UDinoOnlineSession::UpdateHostedSettings(bool Teams,int32 Capacity,bool Bots)
{
    if(!bHosting||!Sessions)return;auto* Current=Sessions->GetSessionSettings(NAME_GameSession);if(!Current)return;
    FOnlineSessionSettings S=*Current;S.NumPublicConnections=bPublic?Capacity:0;S.NumPrivateConnections=bPublic?0:Capacity;
    S.Set(ModeKey,Teams?FString(TEXT("Team Battle")):FString(TEXT("Free-for-All")),EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(BotsKey,Bots,EOnlineDataAdvertisementType::ViaOnlineService);Sessions->UpdateSession(NAME_GameSession,S,true);
}
void UDinoOnlineSession::OnNetworkFailure(UWorld* World,UNetDriver*,ENetworkFailure::Type,const FString& Error)
{
    if((World&&World->GetGameInstance()!=GetGameInstance())||bLeaving)return;
    const FString Reason=Error.Contains(TEXT("Different game version"))?TEXT("Different game version."):Error.Contains(TEXT("Lobby is full"))?TEXT("Lobby is full."):Error.Contains(TEXT("Match is ending"))?TEXT("Match is ending."):bHosting?TEXT("Multiplayer connection failed."):bConnected?TEXT("Host disconnected."):TEXT("Could not connect to host.");
    bBusy=false;Operation=EOperation::None;Leave(Reason);
}
void UDinoOnlineSession::OnTravelFailure(UWorld* World,ETravelFailure::Type,const FString&)
{if(World&&World->GetGameInstance()!=GetGameInstance())return;if(bInSession){bBusy=false;Operation=EOperation::None;Leave(TEXT("Could not load the multiplayer match."));}}
