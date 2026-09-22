#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UHealthComponent();
    virtual void TickComponent(float Dt,ELevelTick TickType,FActorComponentTickFunction* Tick) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Replicated) float Current=1000;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Replicated) float Maximum=1000;
    UPROPERTY(Replicated) float RegenDelay=5;
    UPROPERTY(Replicated) float  RegenRate=.02f;
    UPROPERTY(Replicated) float  LastDamageTime=-100;
    UPROPERTY(Replicated) float  HitFlash=0;
    bool bInvulnerable=false;
    void Reset(float Max,float Delay,float Rate);
    float Receive(float Amount);
    void Heal(float Amount);
    float Fraction() const { return Maximum>0 ? Current/Maximum:0; }
    bool IsDead() const { return Current<=0; }
    float MovementFactor() const { return Fraction()<.25f?.70f:(Fraction()<.5f?.85f:1.f); }
    float AttackSpeedFactor() const { return Fraction()<.25f?.75f:(Fraction()<.5f?.9f:1.f); }
};
