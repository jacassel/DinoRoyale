#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HungerComponent.generated.h"

UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UHungerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UHungerComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    float Current=100,Maximum=100;
    float Fraction() const {return Current/FMath::Max(1.f,Maximum);}
    void Reset();
    void Restore(float Amount);
    float HealthRegenFactor() const;
    float StaminaRegenFactor() const;
};
