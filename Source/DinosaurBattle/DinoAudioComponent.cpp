#include "DinoAudioComponent.h"
#include "DinosaurCharacter.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"

UDinoAudioComponent::UDinoAudioComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UDinoAudioComponent::ResetAudio()
{
    for(auto Voice:Voices)if(IsValid(Voice)){Voice->Stop();Voice->DestroyComponent();}
    Voices.Empty();Ends.Empty();Distance=0;StepCooldown=ImpactCooldown=0;Previous=GetOwner()->GetActorLocation();
}
void UDinoAudioComponent::EndPlay(const EEndPlayReason::Type Reason){ResetAudio();Super::EndPlay(Reason);}
void UDinoAudioComponent::PlayEvent(int32 Kind)
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return;
    if(Kind==3&&ImpactCooldown>0)return;
    if(Kind==4)ResetAudio();
    // Cull inaudible sources and cap per-animal overlap. Combat has priority over steps.
    const FVector Listener=UGameplayStatics::GetPlayerCameraManager(this,0)?UGameplayStatics::GetPlayerCameraManager(this,0)->GetCameraLocation():D->GetActorLocation();
    if(FVector::DistSquared(Listener,D->GetActorLocation())>FMath::Square(6500.f))return;
    int32 Total=0;for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->Audio)Total+=It->Audio->ActiveVoices();
    if((Kind==0&&Total>=8)||(Kind!=4&&Total>=20))return;
    if(Voices.Num()>=2){if(Kind==0)return;Voices[0]->Stop();Voices[0]->DestroyComponent();Voices.RemoveAt(0);Ends.RemoveAt(0);}
    const bool Small=D->Species==1||D->Species==3;
    const float Length=Kind==0?.14f:Kind==1?.25f:Kind==2?.52f:Kind==3?.16f:1.25f;
    const float Base=Small?180.f:D->Species==2?65.f:48.f;
    constexpr int32 Rate=22050;const int32 Count=FMath::CeilToInt(Length*Rate);
    TArray<int16> PCM;PCM.SetNumUninitialized(Count);FRandomStream Noise(117+D->Species*31+Kind*97+Steps);
    float Low=0,Phase=0;
    for(int32 I=0;I<Count;++I)
    {
        float T=float(I)/Rate,U=T/Length,N=Noise.FRandRange(-1,1);Low+=.18f*(N-Low);
        float Frequency=Base*(Kind==4?1.5f-.95f*U:Kind==2?1.3f-.55f*U:1.f);
        Phase+=2*PI*Frequency/Rate;
        float Tone=FMath::Sin(Phase)+.25f*FMath::Sin(Phase*2.03f);
        float Envelope=FMath::Min(T/.008f,1.f)*FMath::Pow(1-U,Kind==4?1.2f:2.5f);
        float Sample=Kind==0?(Tone*.4f+Low*.7f):Kind==3?(Low*1.3f+Tone*.2f):(Tone*.48f+Low*.8f)*(1+.15f*FMath::Sin(T*48));
        PCM[I]=int16(FMath::Clamp(Sample*Envelope,-1.f,1.f)*15000);
    }
    auto* Wave=NewObject<USoundWaveProcedural>(this);Wave->SetSampleRate(Rate);Wave->NumChannels=1;Wave->Duration=Length;Wave->bLooping=false;
    Wave->QueueAudio(reinterpret_cast<const uint8*>(PCM.GetData()),PCM.Num()*sizeof(int16));
    auto* Voice=NewObject<UAudioComponent>(D);Voice->bAutoActivate=false;Voice->bAutoDestroy=false;Voice->bIsUISound=false;
    Voice->SetupAttachment(D->GetRootComponent());Voice->RegisterComponent();Voice->SetSound(Wave);
    Voice->bOverrideAttenuation=true;Voice->AttenuationOverrides.bAttenuate=true;Voice->AttenuationOverrides.bSpatialize=true;
    Voice->AttenuationOverrides.AttenuationShapeExtents=FVector(500);Voice->AttenuationOverrides.FalloffDistance=5500;
    Voice->SetVolumeMultiplier(Kind==0?.24f:Kind==3?.4f:Kind==2?.65f:Kind==4?.75f:.4f);
    Voices.Add(Voice);Ends.Add(GetWorld()->GetTimeSeconds()+Length+.05f);Voice->Play();
    if(Kind==0)++Steps;else if(Kind==1)++QuickSounds;else if(Kind==2)++HeavySounds;else if(Kind==3){++Impacts;ImpactCooldown=.12f;}else ++Deaths;
}
void UDinoAudioComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return;
    for(int32 I=Voices.Num()-1;I>=0;--I)if(GetWorld()->GetTimeSeconds()>=Ends[I]){Voices[I]->Stop();Voices[I]->DestroyComponent();Voices.RemoveAt(I);Ends.RemoveAt(I);}
    StepCooldown=FMath::Max(0.f,StepCooldown-Dt);ImpactCooldown=FMath::Max(0.f,ImpactCooldown-Dt);
    const FVector P=D->GetActorLocation();const float Travel=FVector::Dist2D(P,Previous);Previous=P;
    if(D->bDead||D->bInWater||!D->GetCharacterMovement()->IsMovingOnGround()||Travel>500||D->GetVelocity().Size2D()<50){Distance=0;return;}
    Distance+=Travel;const float Stride=D->Species==1?370:D->Species==2?420:650;
    if(Distance>=Stride&&StepCooldown<=0){Distance=0;StepCooldown=D->Species==1?.13f:.22f;PlayEvent(0);}
}
