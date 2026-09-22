#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HungerComponent.generated.h"

UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UHungerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UHungerComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    UPROPERTY(Replicated) float Current=100;
    UPROPERTY(Replicated) float Maximum=100;
    float Fraction() const {return Current/FMath::Max(1.f,Maximum);}
    void Reset();
    void Restore(float Amount);
    float HealthRegenFactor() const;
    float StaminaRegenFactor() const;
};
