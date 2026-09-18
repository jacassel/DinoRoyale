#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "FoodSystem.generated.h"
class UStaticMeshComponent;

UCLASS()
class DINOSAURBATTLE_API AFoodPlant : public AActor
{
    GENERATED_BODY()
public:
    AFoodPlant();
    virtual void Tick(float Dt) override;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Visual;
    float Nutrition=1,RegrowTimer=0;
    bool IsAvailable() const {return Nutrition>0;}
    void Consume(float Amount);
};

UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UFoodInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFoodInteractionComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool StartEating();
    void StopEating();
    AActor* FindFood(float Range=800) const;
    bool bEating=false;
    float EatRate=.12f,FoodConsumed=0;
    TWeakObjectPtr<AActor> Source;
};
