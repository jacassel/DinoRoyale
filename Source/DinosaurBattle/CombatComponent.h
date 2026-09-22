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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UCombatComponent();
    virtual void TickComponent(float Dt,ELevelTick TickType,FActorComponentTickFunction* Tick) override;
    bool QuickAttack();
    bool StartCharge();
    bool ReleaseCharge();
    void Cancel();
    bool SetBrace(bool Active);
    float ChargeFraction() const;
    bool IsBusy() const {return RecoveryLeft>0;}
    UPROPERTY(Replicated) bool bBracing=false;
    UPROPERTY(Replicated) bool bCharging=false;
    UPROPERTY(Replicated) bool bChargedAttack=false;
    UPROPERTY(Replicated) float ChargeElapsed=0;
    UPROPERTY(Replicated) float  RecoveryLeft=0;
    UPROPERTY(Replicated) float  AttackElapsed=0;
    UPROPERTY(Replicated) float  AttackDuration=.7f;
    UPROPERTY(Replicated) float LastDealtDamage=0;
    UPROPERTY(Replicated) int32 TotalHits=0;
    UPROPERTY(Replicated) int32 AttackSerial=0;
    UPROPERTY(Replicated) int32 ComboCount=0;
    float ComboResetLeft=0,BufferedQuick=0;
    UPROPERTY(Replicated) bool bWeakAttack=false;
    float CurrentHeavyPower=0;
    FVector CommitDirection=FVector::ForwardVector;
    TSet<TWeakObjectPtr<ADinosaurCharacter>> HitActors;
private:
    bool bHitPending=false;
    float PendingDamage=0,HitTime=0;
    ADinosaurCharacter* Dino() const;
    void Execute(bool Charged,float Power);
    void DetectHits();
};
