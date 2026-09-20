#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinoAudioComponent.generated.h"
class UAudioComponent;
/** Short original synthesized effects. No assets, loops, or external services. */
UCLASS()
class DINOSAURBATTLE_API UDinoAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UDinoAudioComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void PlayEvent(int32 Kind); // 0 step, 1 quick motion, 2 heavy motion, 3 impact, 4 death
    void ResetAudio();
    int32 Steps=0,QuickSounds=0,HeavySounds=0,Impacts=0,Deaths=0;
    int32 ActiveVoices() const {return Voices.Num();}
private:
    UPROPERTY(Transient) TArray<TObjectPtr<UAudioComponent>> Voices;
    TArray<float> Ends;
    FVector Previous=FVector::ZeroVector;
    float Distance=0,StepCooldown=0,ImpactCooldown=0;
};
