#include "DinoGameMode.h"
#include "DinoPlayerState.h"
#include "DinosaurCharacter.h"
#include "DinosaurAIController.h"
#include "LostValleyWorld.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

FVector ADinoGameMode::ParticipantHome(int32 ID,int32 Team) const
{
    const float Angle=ID*2*PI/FMath::Max(2,MaxParticipants);
    return bTeamMatch?FVector(Team==0?-6000:6000,(ID%5-2)*1500,0):FVector(FMath::Cos(Angle)*6500,FMath::Sin(Angle)*6500,0);
}
void ADinoGameMode::RemoveParticipant(ADinosaurCharacter* D)
{
    if(!D)return;
    if(!D->bPackFollower)
        for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
            if(It->bPackFollower&&It->PackLeaderID==D->CombatantID)RemoveParticipant(*It);
    const int32 ID=D->CombatantID;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)It->DamageContributors.Remove(ID);
    if(auto* AI=Cast<ADinosaurAIController>(D->GetController())){AI->UnPossess();AI->Destroy();}
    Scores.Remove(ID);D->Destroy();
}

void ADinoGameMode::RebuildPacks()
{
    if(GetNetMode()==NM_Standalone)return;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bPackFollower)RemoveParticipant(*It);
    SynchronizePacks();
}
void ADinoGameMode::SynchronizePacks()
{
    if(GetNetMode()==NM_Standalone||bSynchronizingPacks||!HasActorBegunPlay())return;
    TGuardValue<bool> Guard(bSynchronizingPacks,true);
    TArray<ADinosaurCharacter*> Leaders;
    TMap<int32,ADinosaurCharacter*> Followers;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* D=*It;
        if(!D->bPackFollower)
        {
            D->PackLeaderID=D->bMajor&&D->Species==1?D->CombatantID:-1;
            if(D->PackLeaderID>=0&&!D->bDead)Leaders.Add(D);
        }
    }
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bPackFollower)
    {
        auto* Leader=FindCombatant(It->PackLeaderID);
        if(!Leader||Leader->bDead||Leader->Species!=1||Followers.Contains(It->CombatantID))RemoveParticipant(*It);
        else Followers.Add(It->CombatantID,*It);
    }
    for(auto* Leader:Leaders)for(int32 Index=0;Index<2;++Index)
    {
        const int32 ID=1000+Leader->CombatantID*2+Index;
        auto* Follower=Followers.FindRef(ID);
        if(!Follower)
        {
            FVector P=Leader->GetActorLocation()-Leader->GetActorForwardVector()*650+Leader->GetActorRightVector()*(Index==0?-550:550);
            P.Z=ALostValleyWorld::HeightAt(P.X,P.Y)+FSpeciesData::Get(1).HalfHeight+30;
            const FTransform Spawn(Leader->GetActorRotation(),P);
            Follower=GetWorld()->SpawnActorDeferred<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),Spawn,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
            if(!Follower)continue;
            Follower->Species=1;Follower->bPackFollower=true;Follower->bScoringParticipant=false;
            Follower->PackLeaderID=Leader->CombatantID;Follower->CombatantID=ID;Follower->TeamID=Leader->TeamID;
            UGameplayStatics::FinishSpawningActor(Follower,Spawn);
            Follower->HomePosition=P;Follower->bDead=true;Follower->ResetLife();
            auto* AI=GetWorld()->SpawnActor<ADinosaurAIController>();AI->Possess(Follower);
        }
        Follower->TeamID=Leader->TeamID;Follower->bScoringParticipant=false;Follower->ForceNetUpdate();
    }
}
void ADinoGameMode::ReconcileBots()
{
    if(GetNetMode()==NM_Standalone||!HasActorBegunPlay())return;
    TSet<int32> HumanIDs;int32 TeamCounts[2]={0,0};
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)if(auto* PS=It->Get()->GetPlayerState<ADinoPlayerState>())
    {
        HumanIDs.Add(PS->CombatantID);
        if(auto* D=Cast<ADinosaurCharacter>(It->Get()->GetPawn()))D->TeamID=bTeamMatch?PS->TeamID:-1;
        if(PS->TeamID>=0&&PS->TeamID<2)++TeamCounts[PS->TeamID];
    }
    TMap<int32,ADinosaurCharacter*> Bots;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bFillerBot)
    {
        const int32 ID=It->CombatantID;
        if(!bFillBots||ID<0||ID>=MaxParticipants||HumanIDs.Contains(ID)||Bots.Contains(ID))RemoveParticipant(*It);
        else Bots.Add(ID,*It);
    }
    if(!bFillBots)return;
    for(int32 ID=0;ID<MaxParticipants;++ID)
    {
        if(HumanIDs.Contains(ID))continue;
        const int32 Team=bTeamMatch?(TeamCounts[0]<=TeamCounts[1]?0:1):-1;
        if(Team>=0)++TeamCounts[Team];
        auto* D=Bots.FindRef(ID);
        if(!D)
        {
            const int32 Species=ID%3;FVector P=ParticipantHome(ID,Team);
            P.Z=ALostValleyWorld::HeightAt(P.X,P.Y)+FSpeciesData::Get(Species).HalfHeight+30;
            const FTransform Spawn(FRotator::ZeroRotator,P);
            D=GetWorld()->SpawnActorDeferred<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),Spawn,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
            if(!D)continue;
            D->Species=Species;D->CombatantID=ID;D->TeamID=Team;D->bFillerBot=true;D->bScoringParticipant=true;
            UGameplayStatics::FinishSpawningActor(D,Spawn);
            D->HomePosition=ParticipantHome(ID,Team);D->bDead=true;D->ResetLife();
            auto* AI=GetWorld()->SpawnActor<ADinosaurAIController>();AI->Possess(D);Scores.Add(ID,FDinoScore());
        }
        D->TeamID=Team;D->ForceNetUpdate();
    }
}
