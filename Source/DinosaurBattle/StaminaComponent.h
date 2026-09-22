#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

/** Shared resource rules for human and AI abilities. Exhaustion has hysteresis. */
UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UStaminaComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UStaminaComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    UPROPERTY(Replicated) float Current=100;
    UPROPERTY(Replicated) float Maximum=100;
    UPROPERTY(Replicated) float ExhaustionLeft=0;
    UPROPERTY(Replicated) float RegenDelayLeft=0;
    UPROPERTY(Replicated) bool bExhausted=false;
    float Fraction() const {return Current/FMath::Max(1.f,Maximum);}
    bool CanSpend(float Amount) const {return !bExhausted&&Current>=Amount;}
    bool Spend(float Amount);
    void Drain(float Amount);
    void Restore(float Amount);
    void Reset();
};
