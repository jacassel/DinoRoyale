#include "DinoGameMode.h"
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
ADinoGameMode::ADinoGameMode(){DefaultPawnClass=ADinosaurCharacter::StaticClass();HUDClass=ADinoHUD::StaticClass();PlayerControllerClass=ADinoPlayerController::StaticClass();}
void ADinoGameMode::BeginPlay()
{
    Super::BeginPlay();
    GConfig->GetBool(TEXT("Dino.UserSettings"),TEXT("TeamMode"),bTeamMatch,GGameIni);
    GConfig->GetInt(TEXT("Dino.Match"),TEXT("SoloKillGoal"),SoloKillGoal,GGameIni);
    GConfig->GetInt(TEXT("Dino.Match"),TEXT("TeamKillGoal"),TeamKillGoal,GGameIni);
    GConfig->GetFloat(TEXT("Dino.Match"),TEXT("AssistWindow"),AssistWindow,GGameIni);
    GConfig->GetBool(TEXT("Dino.Match"),TEXT("SharePackKills"),bSharePackKills,GGameIni);
    ALostValleyWorld* Valley=nullptr;
    for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
    if(!Valley)Valley=GetWorld()->SpawnActor<ALostValleyWorld>();
    GetWorld()->SpawnActor<ADinoEffects>();
    auto* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,8000),FRotator(-32,-38,0));
    auto* Light=Cast<UDirectionalLightComponent>(Sun->GetLightComponent());
    Light->SetMobility(EComponentMobility::Movable);Light->SetIntensity(4.f);Light->SetLightColor(FLinearColor(1,.88f,.69f));
    Light->SetAtmosphereSunLight(true);Light->DynamicShadowDistanceMovableLight=20000;Light->DynamicShadowCascades=3;
    GetWorld()->SpawnActor<ASkyAtmosphere>();
    auto* Sky=GetWorld()->SpawnActor<ASkyLight>();Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sky->GetLightComponent()->SourceType=SLS_SpecifiedCubemap;Sky->GetLightComponent()->SetCubemap(LoadObject<UTextureCube>(nullptr,TEXT("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap")));Sky->GetLightComponent()->SetIntensity(1.2f);Sky->GetLightComponent()->SetRealTimeCaptureEnabled(false);
    auto* Fog=GetWorld()->SpawnActor<AExponentialHeightFog>();
    Fog->GetComponent()->SetFogDensity(.007f);Fog->GetComponent()->SetFogHeightFalloff(.18f);
    Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.40f,.56f,.62f));Fog->GetComponent()->SetStartDistance(6000);
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
    for(int32 I=0;I<9;++I)SpawnDino(I/3,Spawns[I]*.5f,I+1,true);
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
    StartRound();
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
    if(bTeamMatch&&A->TeamID>=0&&B->TeamID>=0)return A->TeamID!=B->TeamID;
    return !(A->Species==1&&B->Species==1);
}
ADinosaurCharacter* ADinoGameMode::GetPackLeader(const ADinosaurCharacter* Member) const
{
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
    bTeamMatch=Enabled;GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("TeamMode"),bTeamMatch,GGameIni);GConfig->Flush(false,GGameIni);StartRound();
}
void ADinoGameMode::StartRound()
{
    if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController()))PC->MapPins.Empty();
    bRoundOver=false;WinnerID=WinnerTeam=-1;TeamKills[0]=TeamKills[1]=0;Scores.Empty();++RoundNumber;RoundStartTime=GetWorld()->GetTimeSeconds();
    const FVector SoloHomes[]={FVector(0,0,0),FVector(12500,3000,0),FVector(28500,-6000,0),FVector(-23500,13500,0),FVector(4500,4000,0),FVector(5000,4400,0),FVector(4400,4800,0),FVector(-14000,-22000,0),FVector(8500,-6500,0),FVector(22000,18000,0)};
    const int32 TeamSpecies[]={0,1,1,1,2,0,1,1,1,2};
    const auto* Player=Cast<ADinosaurCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
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
}
void ADinoGameMode::RegisterDamage(ADinosaurCharacter* Victim,ADinosaurCharacter* Attacker,float Amount)
{
    if(!Victim||!Attacker||Amount<=0||!Attacker->bMajor||!AreEnemies(Victim,Attacker))return;
    Victim->DamageContributors.FindOrAdd(Attacker->CombatantID)=GetWorld()->GetTimeSeconds();
}
void ADinoGameMode::RegisterDeath(ADinosaurCharacter* Victim)
{
    if(!Victim||!Victim->bMajor||bRoundOver)return;Scores.FindOrAdd(Victim->CombatantID).Deaths++;
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
        if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController()))PC->SetMenuOpen(true);
    }
}
FString ADinoGameMode::WinnerName() const
{
    if(bTeamMatch)return WinnerTeam==0?TEXT("YOUR TEAM WINS"):TEXT("RIVAL TEAM WINS");
    if(WinnerID==0)return TEXT("YOU WIN");
    auto* D=FindCombatant(WinnerID);return D?D->Stats().Name+TEXT(" WINS"):TEXT("ROUND COMPLETE");
}
