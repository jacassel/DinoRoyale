#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpeciesData.h"
#include "DinosaurCharacter.generated.h"
class UHealthComponent;
class UStaminaComponent;
class UHungerComponent;
class UCombatComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UDinoAnimationComponent;
class UFoodInteractionComponent;
class UDinoAudioComponent;

UCLASS()
class DINOSAURBATTLE_API ADinosaurCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ADinosaurCharacter(const FObjectInitializer& ObjectInitializer=FObjectInitializer::Get());
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UFUNCTION() void OnRep_Species();
    UFUNCTION() void OnRep_Life();
    UFUNCTION(Server,Reliable) void ServerAction(uint8 Action);
    UFUNCTION(NetMulticast,Unreliable) void MulticastSound(int32 Kind);
    UFUNCTION(NetMulticast,Unreliable) void MulticastBlood(FVector Position,FVector Direction,float Damage);
    void PlayCombatSound(int32 Kind);
    bool AcceptsGameplayInput() const;
    UPROPERTY(Replicated) bool bScoringParticipant=true;
    virtual void Tick(float Dt) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) UHealthComponent* Health;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) UCombatComponent* Combat;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) UStaminaComponent* Stamina;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) UHungerComponent* Hunger;
    UPROPERTY(Replicated) float RevealUntil=-100;
    UPROPERTY(Replicated) FVector LastRevealedPosition=FVector::ZeroVector;
    void RevealNoise();
    bool CanSeeDinosaur(const ADinosaurCharacter* Other) const;
    bool MapPositionFor(const ADinosaurCharacter* Other,FVector& Position) const;
    bool bSprintRequested=false;
    UPROPERTY(Replicated) bool bSprinting=false;
    void SprintOn(){bSprintRequested=true;}
    void SprintOff(){bSprintRequested=false;bSprinting=false;}
    float TurnFactor() const;
    UPROPERTY(VisibleAnywhere) UDinoAnimationComponent* Animation;
    UPROPERTY(VisibleAnywhere) UFoodInteractionComponent* Food;
    UPROPERTY(VisibleAnywhere) UDinoAudioComponent* Audio;
    UPROPERTY(VisibleAnywhere) USpringArmComponent* CameraBoom;
    UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Placeholder;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,ReplicatedUsing=OnRep_Species) int32 Species=0;
    UPROPERTY(ReplicatedUsing=OnRep_Life) bool bDead=false;
    UPROPERTY(Replicated) bool bMajor=true;
    UPROPERTY(Replicated) bool bInWater=false;
    UPROPERTY(Replicated) bool bSwimming=false;
    float WaterSurface=0,SwimSpeedMultiplier=.50f;
    float WaterSpeedMultiplier=.65f;
    UPROPERTY(Replicated) float DeathTime=0;
    float MouseSensitivity=1.f;
    UPROPERTY(Replicated) int32 CombatantID=0;
    UPROPERTY(Replicated) int32 TeamID=-1;
    TMap<int32,float> DamageContributors;
    float RespawnDelay=10;
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
