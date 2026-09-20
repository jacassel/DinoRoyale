#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurAIController.generated.h"
class ADinosaurCharacter;
class ALostValleyWorld;

UCLASS()
class DINOSAURBATTLE_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()
public:
    ADinosaurAIController();
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float Dt) override;
    void GoTo(const FVector& Point);
    void SetTravelGoal(const FVector& Point);
    void ClearTravelGoal();
    void ResetTactics();
    void SetTestSeed(int32 Seed){Random.Initialize(Seed);}
    void Alert(ADinosaurCharacter* Attacker);
    int32 Personality=3; // 0 aggressive, 1 defensive, 2 skirmisher, 3 balanced
    float RepositionUntil=0,NextReposition=0,Temperament=1;
    FString State=TEXT("Roaming");
    bool bPaused=false,bForcedTravel=false;
    int32 StuckRecoveries=0,FailedPaths=0,AttacksMade=0;
    float DistanceTravelled=0;
    float FightConfidence=.5f,EscapeConfidence=.5f;
    int32 GuardsUsed=0,RetreatDecisions=0,Retaliations=0;
    FString Decision=TEXT("Explore");
    TWeakObjectPtr<ADinosaurCharacter> Target;
    TWeakObjectPtr<ADinosaurCharacter> Leader;
    FVector Goal=FVector::ZeroVector;
private:
    UPROPERTY() ALostValleyWorld* Valley=nullptr;
    TArray<FVector> Path;
    int32 PathIndex=0;
    float ThinkTimer=0,RoamTimer=0,PathTimer=0,StuckTime=0,BraceTime=0,ChargeTarget=0;
    FVector LastLocation=FVector::ZeroVector,LastProgress=FVector::ZeroVector;
    FRandomStream Random;
    TWeakObjectPtr<ADinosaurCharacter> RecentAttacker;
    float RetaliationUntil=0,RetreatUntil=0,NextGuardTime=0,NextTargetReview=0;
    void ChooseRetreat(const ADinosaurCharacter* Threat);
    void Think(float Dt);
    void Steer(float Dt);
    ADinosaurCharacter* Dino() const;
    ADinosaurCharacter* SelectEnemy() const;
    ADinosaurCharacter* PackLeader() const;
};
