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
    const int32 ID=D->CombatantID;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)It->DamageContributors.Remove(ID);
    if(auto* AI=Cast<ADinosaurAIController>(D->GetController())){AI->UnPossess();AI->Destroy();}
    Scores.Remove(ID);D->Destroy();
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
