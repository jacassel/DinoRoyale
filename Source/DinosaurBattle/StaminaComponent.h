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
    UStaminaComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    float Current=100,Maximum=100,ExhaustionLeft=0,RegenDelayLeft=0;
    bool bExhausted=false;
    float Fraction() const {return Current/FMath::Max(1.f,Maximum);}
    bool CanSpend(float Amount) const {return !bExhausted&&Current>=Amount;}
    bool Spend(float Amount);
    void Drain(float Amount);
    void Restore(float Amount);
    void Reset();
};
