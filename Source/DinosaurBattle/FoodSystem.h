#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "FoodSystem.generated.h"
class UStaticMeshComponent;
class USkeletalMeshComponent;
class ADinosaurCharacter;

UCLASS()
class DINOSAURBATTLE_API AFoodPlant : public AActor
{
    GENERATED_BODY()
public:
    AFoodPlant();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UFUNCTION() void OnRep_Nutrition();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Visual;
    UPROPERTY(ReplicatedUsing=OnRep_Nutrition) float Nutrition=120;
    UPROPERTY(Replicated) float MaximumNutrition=120;
    float RegrowTimer=0,RegrowSeconds=120;
    bool IsAvailable() const {return Nutrition>0;}
    float Consume(float Amount);
};

/** Food lifetime is independent of the living combatant's ten-second respawn. */
UCLASS()
class DINOSAURBATTLE_API ADinosaurCarcass : public AActor
{
    GENERATED_BODY()
public:
    ADinosaurCarcass();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UFUNCTION() void OnRep_Carcass();
    UPROPERTY(ReplicatedUsing=OnRep_Carcass) FTransform BodyTransform;
    void Initialize(ADinosaurCharacter* Source);
    UPROPERTY(VisibleAnywhere) USkeletalMeshComponent* Body;
    UPROPERTY(Replicated) float Nutrition=0;
    UPROPERTY(Replicated) float MaximumNutrition=0;
    UPROPERTY(ReplicatedUsing=OnRep_Carcass) int32 Species=3;
    UPROPERTY(Replicated) int32 SourceID=-1;
    float Consume(float Amount);
};

UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UFoodInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFoodInteractionComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool StartEating();
    void StopEating();
    AActor* FindFood(float Range=800) const;
    UPROPERTY(Replicated) bool bEating=false;
    float EatRate=.12f,FoodConsumed=0;
    TWeakObjectPtr<AActor> Source;
};
