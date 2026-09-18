#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"
class ADinosaurCharacter;

UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UCombatComponent();
    virtual void TickComponent(float Dt,ELevelTick TickType,FActorComponentTickFunction* Tick) override;
    bool QuickAttack();
    bool StartCharge();
    bool ReleaseCharge();
    void Cancel();
    bool SetBrace(bool Active);
    float ChargeFraction() const;
    bool IsBusy() const {return RecoveryLeft>0;}
    bool bBracing=false,bCharging=false,bChargedAttack=false;
    float ChargeElapsed=0, RecoveryLeft=0, AttackElapsed=0, AttackDuration=.7f;
    float LastDealtDamage=0;
    int32 TotalHits=0;
private:
    bool bHitPending=false;
    float PendingDamage=0,HitTime=0;
    ADinosaurCharacter* Dino() const;
    void Execute(bool Charged,float Power);
    void DetectHits();
};
