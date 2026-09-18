#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinoAnimationComponent.generated.h"
class UAnimSequence;

/** Explicit finite animation states; gameplay never waits for an animation notify to unlock. */
UCLASS(ClassGroup=(Dinosaur),meta=(BlueprintSpawnableComponent))
class DINOSAURBATTLE_API UDinoAnimationComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UDinoAnimationComponent();
    virtual void TickComponent(float Dt,ELevelTick T,FActorComponentTickFunction* F) override;
    void LoadSpecies();
    FString State=TEXT("Idle");
private:
    UPROPERTY() TMap<FString,UAnimSequence*> Clips;
    int32 LastAttackSerial=-1;
    void Play(const FString& Clip,bool Loop,float Rate=1,bool Restart=false);
};
