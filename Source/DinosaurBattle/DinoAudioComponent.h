#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinoAudioComponent.generated.h"
class UAudioComponent;
class USoundWave;
/** Finite CC0 creature layers, positional foley and state-aware breathing. */
UCLASS()
class DINOSAURBATTLE_API UDinoAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UDinoAudioComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void PlayEvent(int32 Kind); // 0 step, 1 quick, 2 heavy, 3 impact, 4 death, 5 charge, 6 hurt, 7 exertion, 8 injury
    void ResetAudio();
    int32 Steps=0,QuickSounds=0,HeavySounds=0,Impacts=0,Deaths=0;
    int32 Charges=0,Hurts=0,SprintBreaths=0,InjuredBreaths=0,LoadedClips=0;
    int32 ActiveVoices() const {return Voices.Num();}
private:
    UPROPERTY(Transient) TArray<TObjectPtr<UAudioComponent>> Voices;
    UPROPERTY(Transient) TArray<TObjectPtr<USoundWave>> SoundBank;
    TArray<float> Ends;
    TArray<int32> Kinds;
    void RemoveVoice(int32 Index);
    void StopKind(int32 Kind);
    FRandomStream Variation;
    int32 LastVariants[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};
    FVector Previous=FVector::ZeroVector;
    float Distance=0,StepCooldown=0,ImpactCooldown=0,HurtCooldown=0,BreathCooldown=0;
};
