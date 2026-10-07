#include "DinoGameMode.h"
#include "GameFramework/GameSession.h"
#include "TimerManager.h"
#include "DinoOnlineSession.h"
#include "DinoOnlineDiagnostics.h"
#include "DinoGameState.h"
#include "DinoPlayerState.h"
#include "DinosaurCharacter.h"
#include "DinosaurAIController.h"
#include "LostValleyWorld.h"
#include "FoodSystem.h"
#include "DinoHUD.h"
#include "DinoEffects.h"
#include "DinoPlayerController.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/TextureCube.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/CharacterMovementComponent.h"
ADinoGameMode::ADinoGameMode(){GameStateClass=ADinoGameState::StaticClass();PlayerStateClass=ADinoPlayerState::StaticClass();DefaultPawnClass=ADinosaurCharacter::StaticClass();HUDClass=ADinoHUD::StaticClass();PlayerControllerClass=ADinoPlayerController::StaticClass();}
void ADinoGameMode::BeginPlay()
{
    Super::BeginPlay();
    if(GetNetMode()==NM_Standalone)GConfig->GetBool(TEXT("Dino.UserSettings"),TEXT("TeamMode"),bTeamMatch,GGameIni);
    GConfig->GetInt(TEXT("Dino.Match"),TEXT("SoloKillGoal"),SoloKillGoal,GGameIni);
    GConfig->GetInt(TEXT("Dino.Match"),TEXT("TeamKillGoal"),TeamKillGoal,GGameIni);
    if(bOnlineMatch)TeamKillGoal=UGameplayStatics::GetIntOption(OptionsString,TEXT("TeamGoal"),10);
    if(TeamKillGoal!=5&&TeamKillGoal!=10&&TeamKillGoal!=15)TeamKillGoal=10;
    GConfig->GetFloat(TEXT("Dino.Match"),TEXT("AssistWindow"),AssistWindow,GGameIni);
    GConfig->GetBool(TEXT("Dino.Match"),TEXT("SharePackKills"),bSharePackKills,GGameIni);
    ALostValleyWorld::EnsureLocalScene(GetWorld());
    if(GetNetMode()==NM_Standalone)GConfig->GetBool(TEXT("Dino.UserSettings"),TEXT("PerformanceMap"),bPerformanceMap,GGameIni);
    SetMapVariant(bPerformanceMap);
    ALostValleyWorld* Valley=nullptr;for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
    auto SpawnDino=[&](int32 Species,FVector P,int32 ID,bool Major)
    {
        P=Valley->NearestWalkable(P);P.Z+=FSpeciesData::Get(Species).HalfHeight+20;
        FTransform T(FRotator(0,ID*37,0),P);
        auto* D=GetWorld()->SpawnActorDeferred<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        D->Species=Species;D->CombatantID=ID;D->bMajor=Major;
        UGameplayStatics::FinishSpawningActor(D,T);D->HomePosition=D->GetActorLocation();
        auto* AI=GetWorld()->SpawnActor<ADinosaurAIController>();AI->Possess(D);
        return D;
    };
    FRandomStream Random(7512);
    for(int32 I=0;I<18;++I)
    {
        FVector P=ALostValleyWorld::Landmarks()[I%6]+FVector(Random.FRandRange(-3250,3250),Random.FRandRange(-3250,3250),0);
        if(P.Size2D()<2700)P.X+=4000;
        SpawnDino(3,P,100+I,false);
    }
    for(const FVector& Spot:Valley->FoodSpawnPoints)
    {
        GetWorld()->SpawnActor<AFoodPlant>(Spot,FRotator(0,Random.FRandRange(0,360),0));
    }
    for(const FVector& Spot:Valley->TreeFoodSpawnPoints)
        GetWorld()->SpawnActor<AFoodTree>(Spot,FRotator::ZeroRotator);
    auto* PC=GetWorld()->GetFirstPlayerController();if(PC)
    {
        PC->SetControlRotation(FRotator(-13,0,0));PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=25;
        if(auto* D=Cast<ADinosaurCharacter>(PC->GetPawn())){D->SetActorLocation(Valley->GroundPoint(0,0,D->Stats().HalfHeight+25));D->HomePosition=D->GetActorLocation();}
    }
    if(bOnlineMatch)UpdateLobby();else {bFillBots=true;StartRound();}
}

FDinoScore ADinoGameMode::GetScore(int32 ID) const{if(const auto* S=Scores.Find(ID))return *S;return FDinoScore();}
FString ADinoGameMode::MatchName() const{return bTeamMatch?FString::Printf(TEXT("TEAM BATTLE  /  FIRST TO %d POINTS"),TeamKillGoal):FString::Printf(TEXT("SOLO FREE-FOR-ALL  /  FIRST TO %d POINTS"),SoloKillGoal);}
ADinosaurCharacter* ADinoGameMode::FindCombatant(int32 ID) const
{
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor&&It->CombatantID==ID)return *It;return nullptr;
}
bool ADinoGameMode::AreEnemies(const ADinosaurCharacter* A,const ADinosaurCharacter* B) const
{
    if(!A||!B||A==B)return false;
    if(A->Species==3||B->Species==3)return A->Species!=B->Species;
    if(A->PackLeaderID>=0&&A->PackLeaderID==B->PackLeaderID)return false;
    return bTeamMatch&&A->TeamID>=0&&B->TeamID>=0?A->TeamID!=B->TeamID:true;
}
ADinosaurCharacter* ADinoGameMode::GetPackLeader(const ADinosaurCharacter* Member) const
{
    return Member?(Member->bPackFollower?FindCombatant(Member->PackLeaderID):const_cast<ADinosaurCharacter*>(Member)):nullptr;
}
ADinosaurCharacter* ADinoGameMode::ScoringOwner(ADinosaurCharacter* D) const{return D&&FSpeciesData::IsPack(D->Species)&&bSharePackKills?GetPackLeader(D):D;}
bool ADinoGameMode::IsScoringTarget(const ADinosaurCharacter* D) const{return D&&D->bMajor&&(!FSpeciesData::IsPack(D->Species)||GetPackLeader(D)==D);}
void ADinoGameMode::SetTeamMode(bool Enabled)
{
    if(GetNetMode()!=NM_Standalone)return;
    bTeamMatch=Enabled;if(auto* PC=GetWorld()->GetFirstPlayerController())if(auto* PS=PC->GetPlayerState<ADinoPlayerState>())PS->TeamID=Enabled?0:-1;GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("TeamMode"),bTeamMatch,GGameIni);GConfig->Flush(false,GGameIni);StartRound();
}
void ADinoGameMode::StartRound()
{
    if(bOnlineMatch){StartNetworkRound();return;}
    if(bTeamMatch)if(auto* PC=GetWorld()->GetFirstPlayerController())if(auto* PS=PC->GetPlayerState<ADinoPlayerState>())if(PS->TeamID<0)PS->TeamID=0;
    const FString Warning=ValidateSetup();
    if(!Warning.IsEmpty()){if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController())){PC->LobbyStatus=Warning;PC->bMatchSetupOpen=true;PC->SetMenuOpen(true);}return;}
    bOfflineRulesDirty=false;
    if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController()))PC->MapPins.Empty();
    bRoundOver=false;WinnerID=WinnerTeam=-1;TeamKills[0]=TeamKills[1]=0;TeamAssists[0]=TeamAssists[1]=0;Scores.Empty();++RoundNumber;RoundStartTime=GetWorld()->GetTimeSeconds();
    EnforceAllowedSpecies();
    ReconcileBots();
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* D=*It;if(!D->bMajor||D->bPackFollower)continue;
        if(D->IsPlayerControlled())if(auto* PS=D->GetPlayerState<ADinoPlayerState>())D->TeamID=bTeamMatch?PS->TeamID:-1;
        D->HomePosition=ParticipantHome(D->CombatantID,D->TeamID);D->bDead=true;D->ResetLife();
        D->GetCharacterMovement()->StopMovementImmediately();D->bScoringParticipant=true;Scores.Add(D->CombatantID,FDinoScore());
        if(auto* AI=Cast<ADinosaurAIController>(D->GetController())){AI->ResetTactics();AI->ClearTravelGoal();}
    }
    RebuildPacks();
    if(auto* GS=GetGameState<ADinoGameState>())GS->SynchronizeRules();
}
void ADinoGameMode::RegisterDamage(ADinosaurCharacter* Victim,ADinosaurCharacter* Attacker,float Amount)
{
    if(!Victim||!Attacker||Amount<=0||!Attacker->bMajor||!AreEnemies(Victim,Attacker))return;
    Victim->DamageContributors.FindOrAdd(Attacker->CombatantID)=GetWorld()->GetTimeSeconds();
}
void ADinoGameMode::RegisterDeath(ADinosaurCharacter* Victim)
{
    if(!HasAuthority()||!Victim||!Victim->bMajor||bRoundOver||(GetNetMode()!=NM_Standalone&&Victim->bPackFollower))return;Scores.FindOrAdd(Victim->CombatantID).Deaths++;
    if(!IsScoringTarget(Victim))return;
    auto* ActualKiller=Victim->LastAttacker.Get();auto* Killer=ScoringOwner(ActualKiller);
    const float* LastHit=ActualKiller?Victim->DamageContributors.Find(ActualKiller->CombatantID):nullptr;
    if(!Killer||!Killer->bMajor||!LastHit||GetWorld()->GetTimeSeconds()-*LastHit>AssistWindow||!AreEnemies(Killer,Victim))return;
    Scores.FindOrAdd(Killer->CombatantID).Kills++;
    TSet<int32> AssistIDs;
    for(auto Pair:Victim->DamageContributors)
    {
        auto* Contributor=ScoringOwner(FindCombatant(Pair.Key));
        if(Contributor&&Contributor!=Killer&&AreEnemies(Contributor,Victim)&&GetWorld()->GetTimeSeconds()-Pair.Value<=AssistWindow)AssistIDs.Add(Contributor->CombatantID);
    }
    for(int32 ID:AssistIDs)
    {
        Scores.FindOrAdd(ID).Assists++;
        if(bTeamMatch)if(auto* Contributor=FindCombatant(ID))if(Contributor->TeamID>=0&&Contributor->TeamID<2)++TeamAssists[Contributor->TeamID];
    }
    if(bTeamMatch&&Killer->TeamID>=0&&Killer->TeamID<2)++TeamKills[Killer->TeamID];
    // An assist can cross the win threshold even when another participant lands the kill.
    auto* Winner=Killer;
    if(!bTeamMatch)for(int32 ID:AssistIDs)if(auto* Contributor=FindCombatant(ID))
    {
        const auto A=GetScore(ID),B=GetScore(Winner->CombatantID);
        if(A.SoloPoints()>B.SoloPoints()||(A.SoloPoints()==B.SoloPoints()&&(A.Kills>B.Kills||(A.Kills==B.Kills&&ID<Winner->CombatantID))))Winner=Contributor;
    }
    const bool Won=bTeamMatch?(Killer->TeamID>=0&&TeamPoints(Killer->TeamID)>=TeamKillGoal):GetScore(Winner->CombatantID).SoloPoints()>=SoloKillGoal;
    if(Won&&!bIgnoreWinCondition)
    {
        bRoundOver=true;WinnerID=Winner->CombatantID;WinnerTeam=Winner->TeamID;
        if(auto* GS=GetGameState<ADinoGameState>())GS->SynchronizeRules();
        if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController()))PC->SetMenuOpen(true);
    }
}
FString ADinoGameMode::WinnerName() const
{
    if(bTeamMatch)return WinnerTeam==0?TEXT("YOUR TEAM WINS"):TEXT("RIVAL TEAM WINS");
    if(WinnerID==0)return TEXT("YOU WIN");
    auto* D=FindCombatant(WinnerID);return D?D->Stats().Name+TEXT(" WINS"):TEXT("ROUND COMPLETE");
}

void ADinoGameMode::PostLogin(APlayerController* PC)
{
    if(auto* PS=PC->GetPlayerState<ADinoPlayerState>())
    {
        TSet<int32> Used;
        for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            if(It->Get()!=PC)if(auto* Other=It->Get()->GetPlayerState<ADinoPlayerState>())Used.Add(Other->CombatantID);
        PS->CombatantID=0;while(Used.Contains(PS->CombatantID))++PS->CombatantID;
        PS->SelectedSpecies=MatchRules.Resolve(PS->SelectedSpecies);PS->bHost=PC->IsLocalController();PS->TeamID=bTeamMatch?ChooseTeam(PS->CombatantID):-1;
        if(GetNetMode()!=NM_Standalone)if(auto* Bot=FindCombatant(PS->CombatantID))if(Bot->bFillerBot)RemoveParticipant(Bot);
    }
    // UE starts/possesses the pawn inside Super::PostLogin; assign its slot first.
    Super::PostLogin(PC);
    if(GetNetMode()!=NM_Standalone)
    {
        const auto* PS=PC->GetPlayerState<ADinoPlayerState>();
        UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] PostLogin controller=%s local=%d playerState=%s replicatedState=%d slot=%d players=%d pawn=%d"),
            *PC->GetClass()->GetName(),PC->IsLocalController(),PS?*PS->GetClass()->GetName():TEXT("none"),PS&&PS->GetIsReplicated(),PS?PS->CombatantID:-1,GetNumPlayers(),PC->GetPawn()!=nullptr);
        DinoOnlineDiagnostics::World(GetWorld(),TEXT("PostLogin"));
    }
    if(GetNetMode()!=NM_Standalone)UpdateLobby();
}
void ADinoGameMode::RestartPlayer(AController* C)
{
    Super::RestartPlayer(C);
    if(auto* D=Cast<ADinosaurCharacter>(C->GetPawn()))if(auto* PS=C->GetPlayerState<ADinoPlayerState>())
    {
        D->CombatantID=FMath::Max(0,PS->CombatantID);D->TeamID=PS->TeamID;D->ApplySpecies(PS->SelectedSpecies);
        D->HomePosition=FVector(D->CombatantID*2400.f,0,0);D->bDead=true;D->ResetLife();
        Scores.FindOrAdd(D->CombatantID);
    }
}
APawn* ADinoGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* C,const FTransform& Transform)
{
    if(GetNetMode()==NM_Standalone)return Super::SpawnDefaultPawnAtTransform_Implementation(C,Transform);
    const auto* PS=C->GetPlayerState<ADinoPlayerState>();
    const int32 ID=PS?FMath::Max(0,PS->CombatantID):0;
    const int32 Species=PS?PS->SelectedSpecies:0;
    FVector P(ID*2400.f,0,0);P.Z=ALostValleyWorld::HeightAt(P.X,P.Y)+FSpeciesData::Get(Species).HalfHeight+35;
    FTransform Spawn(FRotator::ZeroRotator,P);
    auto* D=GetWorld()->SpawnActorDeferred<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),Spawn,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if(D){D->Species=Species;D->CombatantID=ID;UGameplayStatics::FinishSpawningActor(D,Spawn);}
    return D;
}
void ADinoGameMode::Logout(AController* C)
{
    if(GetNetMode()!=NM_Standalone)UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] Logout controller=%s playersBeforeCleanup=%d"),C?*C->GetClass()->GetName():TEXT("none"),GetNumPlayers());
    if(auto* D=Cast<ADinosaurCharacter>(C->GetPawn()))RemoveParticipant(D);
    Super::Logout(C);
    if(GetNetMode()!=NM_Standalone)GetWorldTimerManager().SetTimerForNextTick(this,&ADinoGameMode::UpdateLobby);
}
void ADinoGameMode::PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& ID,FString& Error)
{
    if(bOnlineMatch)UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] PreLogin incoming=1 addressPresent=%d identityValid=%d currentPlayers=%d capacity=%d"),!Address.IsEmpty(),ID.IsValid(),GetNumPlayers(),MaxParticipants);
    if(bOnlineMatch&&!UGameplayStatics::HasOption(OptionsString,TEXT("bUseIPSockets")))
    {
        auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>();
        Error=Online&&Online->ValidateIdentity(ID)?GameSession->ApproveLogin(Options):TEXT("Online identity is incompatible.");
    }
    else Super::PreLogin(Options,Address,ID,Error);
    const int32 RemoteBuild=UGameplayStatics::GetIntOption(Options,TEXT("DinoBuild"),0);
    if(bOnlineMatch&&RemoteBuild!=UDinoOnlineSession::BuildVersion)Error=TEXT("Different game version.");
    if(Error.IsEmpty()&&GetNumPlayers()>=MaxParticipants)Error=TEXT("Lobby is full.");
    if(bRoundOver)Error=TEXT("Match is ending.");
    if(bOnlineMatch&&!bLobby&&bTeamMatch&&MatchRules.bExplicitBotCounts&&GetNumPlayers()+1+MatchRules.TeamABots+MatchRules.TeamBBots>MaxParticipants)Error=TEXT("Selected bots and players exceed capacity. Host must return to lobby and adjust bot counts.");
    if(bOnlineMatch)UE_LOG(LogTemp,Display,TEXT("Dino prelogin local=%d remote=%d source=DinoBuild travel option decision=%s reason=%s"),UDinoOnlineSession::BuildVersion,RemoteBuild,Error.IsEmpty()?TEXT("ACCEPT"):TEXT("REJECT"),Error.IsEmpty()?TEXT("compatible"):*Error);
    FGameModeEvents::GameModePreLoginEvent.Broadcast(this,ID,Error);
}
APlayerController* ADinoGameMode::Login(UPlayer* NewPlayer,ENetRole InRemoteRole,const FString& Portal,const FString& Options,const FUniqueNetIdRepl& ID,FString& Error)
{
    auto* PC=Super::Login(NewPlayer,InRemoteRole,Portal,Options,ID,Error);
    if(bOnlineMatch)UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] Login controllerCreated=%d playerStateCreated=%d accepted=%d"),PC!=nullptr,PC&&PC->GetPlayerState<ADinoPlayerState>()!=nullptr,Error.IsEmpty());
    return PC;
}
