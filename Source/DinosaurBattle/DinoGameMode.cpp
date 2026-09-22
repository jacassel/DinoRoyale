#include "DinoGameMode.h"
#include "GameFramework/GameSession.h"
#include "TimerManager.h"
#include "DinoOnlineSession.h"
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
    GConfig->GetFloat(TEXT("Dino.Match"),TEXT("AssistWindow"),AssistWindow,GGameIni);
    GConfig->GetBool(TEXT("Dino.Match"),TEXT("SharePackKills"),bSharePackKills,GGameIni);
    ALostValleyWorld::EnsureLocalScene(GetWorld());
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
    const FVector Spawns[]={FVector(12500,3000,0),FVector(28500,-6000,0),FVector(-23500,13500,0),
        FVector(4500,4000,0),FVector(5000,4400,0),FVector(4400,4800,0),
        FVector(-14000,-22000,0),FVector(8500,-6500,0),FVector(22000,18000,0)};
    if(GetNetMode()==NM_Standalone)for(int32 I=0;I<9;++I)SpawnDino(I/3,Spawns[I]*.5f,I+1,true);
    FRandomStream Random(7512);
    for(int32 I=0;I<18;++I)
    {
        FVector P=ALostValleyWorld::Landmarks()[I%6]+FVector(Random.FRandRange(-3250,3250),Random.FRandRange(-3250,3250),0);
        if(P.Size2D()<2700)P.X+=4000;
        SpawnDino(3,P,100+I,false);
    }
    for(const FVector& P:Valley->FeedingSpots)for(int32 I=0;I<4;++I)
    {
        FVector Spot=Valley->NearestWalkable(P+FVector(I%2*750,I/2*750,0));
        GetWorld()->SpawnActor<AFoodPlant>(Spot,FRotator(0,Random.FRandRange(0,360),0));
    }
    auto* PC=GetWorld()->GetFirstPlayerController();if(PC)
    {
        PC->SetControlRotation(FRotator(-13,0,0));PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=25;
        if(auto* D=Cast<ADinosaurCharacter>(PC->GetPawn())){D->SetActorLocation(Valley->GroundPoint(0,0,D->Stats().HalfHeight+25));D->HomePosition=D->GetActorLocation();}
    }
    if(bOnlineMatch)UpdateLobby();else StartRound();
}

FDinoScore ADinoGameMode::GetScore(int32 ID) const{if(const auto* S=Scores.Find(ID))return *S;return FDinoScore();}
FString ADinoGameMode::MatchName() const{return bTeamMatch?FString::Printf(TEXT("5 v 5 TEAM FIGHT  /  FIRST TO %d"),TeamKillGoal):FString::Printf(TEXT("SOLO FREE-FOR-ALL  /  FIRST TO %d"),SoloKillGoal);}
ADinosaurCharacter* ADinoGameMode::FindCombatant(int32 ID) const
{
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor&&It->CombatantID==ID)return *It;return nullptr;
}
bool ADinoGameMode::AreEnemies(const ADinosaurCharacter* A,const ADinosaurCharacter* B) const
{
    if(!A||!B||A==B)return false;
    if(A->Species==3||B->Species==3)return A->Species!=B->Species;
    if(GetNetMode()!=NM_Standalone)
    {
        if(A->PackLeaderID>=0&&A->PackLeaderID==B->PackLeaderID)return false;
        return bTeamMatch&&A->TeamID>=0&&B->TeamID>=0?A->TeamID!=B->TeamID:true;
    }
    if(bTeamMatch&&A->TeamID>=0&&B->TeamID>=0)return A->TeamID!=B->TeamID;
    return !(A->Species==1&&B->Species==1);
}
ADinosaurCharacter* ADinoGameMode::GetPackLeader(const ADinosaurCharacter* Member) const
{
    if(GetNetMode()!=NM_Standalone)return Member?(Member->bPackFollower?FindCombatant(Member->PackLeaderID):const_cast<ADinosaurCharacter*>(Member)):nullptr;
    if(!Member||Member->Species!=1)return nullptr;ADinosaurCharacter* Leader=nullptr;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* D=*It;if(!D->bMajor||D->Species!=1||(bTeamMatch&&D->TeamID!=Member->TeamID))continue;
        // The role persists through the ten-second respawn; killing followers never becomes a score exploit.
        if(D->IsPlayerControlled())return D;
        if(!Leader||D->CombatantID<Leader->CombatantID)Leader=D;
    }
    return Leader;
}
ADinosaurCharacter* ADinoGameMode::ScoringOwner(ADinosaurCharacter* D) const{return D&&D->Species==1&&bSharePackKills?GetPackLeader(D):D;}
bool ADinoGameMode::IsScoringTarget(const ADinosaurCharacter* D) const{return D&&D->bMajor&&(D->Species!=1||GetPackLeader(D)==D);}
void ADinoGameMode::SetTeamMode(bool Enabled)
{
    if(GetNetMode()!=NM_Standalone)return;
    bTeamMatch=Enabled;GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("TeamMode"),bTeamMatch,GGameIni);GConfig->Flush(false,GGameIni);StartRound();
}
void ADinoGameMode::StartRound()
{
    if(bOnlineMatch){StartNetworkRound();return;}
    if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController()))PC->MapPins.Empty();
    bRoundOver=false;WinnerID=WinnerTeam=-1;TeamKills[0]=TeamKills[1]=0;Scores.Empty();++RoundNumber;RoundStartTime=GetWorld()->GetTimeSeconds();
    const FVector SoloHomes[]={FVector(0,0,0),FVector(12500,3000,0),FVector(28500,-6000,0),FVector(-23500,13500,0),FVector(4500,4000,0),FVector(5000,4400,0),FVector(4400,4800,0),FVector(-14000,-22000,0),FVector(8500,-6500,0),FVector(22000,18000,0)};
    const int32 TeamSpecies[]={0,1,1,1,2,0,1,1,1,2};
    const auto* FirstPC=GetWorld()->GetFirstPlayerController();
    const auto* Player=FirstPC?Cast<ADinosaurCharacter>(FirstPC->GetPawn()):nullptr;
    const bool PlayerRaptor=Player&&Player->Species==1;
    ALostValleyWorld* Valley=nullptr;for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* D=*It;if(!D->bMajor)continue;int32 ID=D->CombatantID;if(ID<0||ID>9)continue;
        D->TeamID=bTeamMatch?(ID<=4?0:1):-1;
        if(!D->IsPlayerControlled())D->ApplySpecies(bTeamMatch?(PlayerRaptor&&ID==1?0:TeamSpecies[ID]):(PlayerRaptor&&ID==6?0:(ID-1)/3));
        FVector Home=SoloHomes[ID]*.5f;
        if(bTeamMatch){int32 Slot=ID<=4?ID:ID-5;Home=FVector(ID<=4?-6000:6000,(Slot-2)*1300,0);}
        if(Valley)Home=Valley->NearestWalkable(Home);
        D->HomePosition=Home;D->bDead=true;D->ResetLife();D->GetCharacterMovement()->StopMovementImmediately();
        D->SetActorRotation(FRotator(0,bTeamMatch&&D->TeamID==1?180:0,0));Scores.Add(ID,FDinoScore());
        if(auto* AI=Cast<ADinosaurAIController>(D->GetController())){AI->ResetTactics();AI->ClearTravelGoal();AI->State=TEXT("Roaming");}
    }
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)It->bScoringParticipant=IsScoringTarget(*It);
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
    for(int32 ID:AssistIDs)Scores.FindOrAdd(ID).Assists++;
    if(bTeamMatch&&Killer->TeamID>=0&&Killer->TeamID<2)++TeamKills[Killer->TeamID];
    bool Won=bTeamMatch?(Killer->TeamID>=0&&TeamKills[Killer->TeamID]>=TeamKillGoal):Scores[Killer->CombatantID].Kills>=SoloKillGoal;
    if(Won&&!bIgnoreWinCondition)
    {
        bRoundOver=true;WinnerID=Killer->CombatantID;WinnerTeam=Killer->TeamID;
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
        PS->bHost=PC->IsLocalController();PS->TeamID=bTeamMatch?ChooseTeam(PS->CombatantID):-1;
        if(GetNetMode()!=NM_Standalone)if(auto* Bot=FindCombatant(PS->CombatantID))if(Bot->bFillerBot)RemoveParticipant(Bot);
    }
    // UE starts/possesses the pawn inside Super::PostLogin; assign its slot first.
    Super::PostLogin(PC);
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
    if(auto* D=Cast<ADinosaurCharacter>(C->GetPawn()))RemoveParticipant(D);
    Super::Logout(C);
    if(GetNetMode()!=NM_Standalone)GetWorldTimerManager().SetTimerForNextTick(this,&ADinoGameMode::UpdateLobby);
}
void ADinoGameMode::PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& ID,FString& Error)
{
    if(bOnlineMatch&&!UGameplayStatics::HasOption(OptionsString,TEXT("bUseIPSockets")))
    {
        auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>();
        Error=Online&&Online->ValidateIdentity(ID)?GameSession->ApproveLogin(Options):TEXT("Online identity is incompatible.");
    }
    else Super::PreLogin(Options,Address,ID,Error);
    if(bOnlineMatch&&UGameplayStatics::GetIntOption(Options,TEXT("DinoBuild"),0)!=UDinoOnlineSession::BuildVersion)Error=TEXT("Different game version.");
    if(Error.IsEmpty()&&GetNumPlayers()>=MaxParticipants)Error=TEXT("Lobby is full.");
    if(bRoundOver)Error=TEXT("Match is ending.");
    FGameModeEvents::GameModePreLoginEvent.Broadcast(this,ID,Error);
}
