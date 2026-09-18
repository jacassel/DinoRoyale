#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpeciesData.h"
#include "DinosaurCharacter.generated.h"
class UHealthComponent;
class UCombatComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UDinoAnimationComponent;
class UFoodInteractionComponent;

UCLASS()
class DINOSAURBATTLE_API ADinosaurCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ADinosaurCharacter(const FObjectInitializer& ObjectInitializer=FObjectInitializer::Get());
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) UHealthComponent* Health;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) UCombatComponent* Combat;
    UPROPERTY(VisibleAnywhere) UDinoAnimationComponent* Animation;
    UPROPERTY(VisibleAnywhere) UFoodInteractionComponent* Food;
    UPROPERTY(VisibleAnywhere) USpringArmComponent* CameraBoom;
    UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Placeholder;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Species=0;
    bool bDead=false,bMajor=true,bInWater=false,bSwimming=false;
    float WaterSurface=0,SwimSpeedMultiplier=.50f;
    float WaterSpeedMultiplier=.65f;
    float DeathTime=0,MouseSensitivity=1.f;
    int32 CombatantID=0,TeamID=-1;
    TMap<int32,float> DamageContributors;
    float Nutrition=1,RespawnDelay=10;
    FVector HomePosition=FVector::ZeroVector;
    TWeakObjectPtr<ADinosaurCharacter> LastAttacker;
    const FSpeciesData& Stats() const {return FSpeciesData::Get(Species);}
    void ApplySpecies(int32 ID);
    void ReceiveHit(float Damage,ADinosaurCharacter* Attacker);
    bool IsEnemy(const ADinosaurCharacter* Other) const;
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void Look(float Value);
    void BeginJump();
    void BraceOn();
    void BraceOff();
    void Quick();
    void ChargeOn();
    void ChargeOff();
    void Eat();
    void StopEating();
    void Die();
    void ResetLife();
    void ChooseRex();
    void ChooseRaptor();
    void ChooseTrike();
};
