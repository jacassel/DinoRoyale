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
    void Alert(ADinosaurCharacter* Attacker);
    FString State=TEXT("Roaming");
    bool bPaused=false,bForcedTravel=false;
    int32 StuckRecoveries=0,FailedPaths=0,AttacksMade=0;
    float DistanceTravelled=0;
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
    void Think(float Dt);
    void Steer(float Dt);
    ADinosaurCharacter* Dino() const;
    ADinosaurCharacter* SelectEnemy() const;
    ADinosaurCharacter* PackLeader() const;
};
