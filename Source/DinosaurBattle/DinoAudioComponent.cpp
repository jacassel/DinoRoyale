#include "DinoAudioComponent.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"

UDinoAudioComponent::UDinoAudioComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UDinoAudioComponent::BeginPlay()
{
    Super::BeginPlay();Variation.Initialize(7139+GetUniqueID()*37);
    const TCHAR* Species[]={TEXT("Trex"),TEXT("Raptor"),TEXT("Trike"),TEXT("Anky"),TEXT("Brachi"),TEXT("Pachy")};
    const TCHAR* Events[]={TEXT("Step"),TEXT("Quick"),TEXT("Heavy"),TEXT("Impact"),TEXT("Death"),TEXT("Charge"),TEXT("Hurt"),TEXT("SprintBreath"),TEXT("InjuredBreath")};
    for(auto Name:Species)for(auto Event:Events)for(int32 V=0;V<3;++V)
    {
        const FString Asset=FString::Printf(TEXT("/Game/Audio/Creatures/%s_%s_%d.%s_%s_%d"),Name,Event,V,Name,Event,V);
        auto* Wave=LoadObject<USoundWave>(nullptr,*Asset);SoundBank.Add(Wave);
        if(Wave)++LoadedClips;else UE_LOG(LogTemp,Error,TEXT("Missing creature audio: %s"),*Asset);
    }
    ResetAudio();
}
void UDinoAudioComponent::RemoveVoice(int32 I)
{
    if(IsValid(Voices[I])){Voices[I]->Stop();Voices[I]->DestroyComponent();}
    Voices.RemoveAt(I);Ends.RemoveAt(I);Kinds.RemoveAt(I);
}
void UDinoAudioComponent::StopKind(int32 Kind)
{
    for(int32 I=Voices.Num()-1;I>=0;--I)if(Kinds[I]==Kind)
    {
        Voices[I]->FadeOut(.065f,0);Ends[I]=FMath::Min(Ends[I],GetWorld()->GetTimeSeconds()+.07f);
        Kinds[I]=-1; // Fade once; repeated ticks must not restart the fade envelope.
    }
}
void UDinoAudioComponent::ResetAudio()
{
    while(Voices.Num())RemoveVoice(Voices.Num()-1);
    Distance=0;StepCooldown=ImpactCooldown=HurtCooldown=0;BreathCooldown=.6f;
    Previous=GetOwner()->GetActorLocation();
}
void UDinoAudioComponent::EndPlay(const EEndPlayReason::Type Reason){ResetAudio();Super::EndPlay(Reason);}
void UDinoAudioComponent::PlayEvent(int32 Kind)
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||Kind<0||Kind>8)return;
    if(D->bDead&&Kind!=4)return;
    if((Kind==3&&ImpactCooldown>0)||(Kind==6&&HurtCooldown>0))return;
    if(Kind==4)ResetAudio();
    const auto* Camera=UGameplayStatics::GetPlayerCameraManager(this,0);
    const FVector Listener=Camera?Camera->GetCameraLocation():D->GetActorLocation();
    const bool Ambient=Kind==0||Kind>=7;
    if(FVector::DistSquared(Listener,D->GetActorLocation())>FMath::Square(Ambient?4200.f:7500.f))return;
    const int32 Species=D->Species==7?0:D->Species==3?1:D->Species>=4?D->Species-1:FMath::Clamp(D->Species,0,2);
    int32 V=(LastVariants[Kind]+1+Variation.RandRange(0,1))%3;LastVariants[Kind]=V;
    const int32 Index=(Species*9+Kind)*3+V;
    if(!SoundBank.IsValidIndex(Index)||!SoundBank[Index])return;
    if(!Ambient){StopKind(7);StopKind(8);BreathCooldown=1.4f;if(Kind!=5)StopKind(5);}
    int32 Total=0;UDinoAudioComponent* Victim=nullptr;int32 VictimIndex=-1;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->Audio)
    {
        auto* A=It->Audio;Total+=A->Voices.Num();
        for(int32 I=0;I<A->Kinds.Num();++I)if(A->Kinds[I]<=0||A->Kinds[I]>=7){Victim=A;VictimIndex=I;}
    }
    if(Ambient&&Total>=12)return;
    if(Total>=24){if(!Ambient&&Victim)Victim->RemoveVoice(VictimIndex);else return;}
    if(Voices.Num()>=3)
    {
        if(Ambient)return;
        int32 Replace=0;for(int32 I=0;I<Kinds.Num();++I)if(Kinds[I]<=0||Kinds[I]>=7){Replace=I;break;}
        RemoveVoice(Replace);
    }
    auto* Wave=SoundBank[Index].Get();
    auto* Voice=NewObject<UAudioComponent>(D);Voice->bAutoActivate=false;Voice->bAutoDestroy=false;Voice->bIsUISound=false;
    Voice->SetupAttachment(D->GetRootComponent());Voice->RegisterComponent();Voice->SetSound(Wave);
    Voice->bOverrideAttenuation=true;Voice->AttenuationOverrides.bAttenuate=true;Voice->AttenuationOverrides.bSpatialize=true;
    Voice->AttenuationOverrides.AttenuationShapeExtents=FVector(Ambient?280:650);
    Voice->AttenuationOverrides.FalloffDistance=Ambient?3600:6500;
    const float Levels[]={.36f,.73f,.92f,.64f,.90f,.60f,.72f,.32f,.37f};
    float Volume=Levels[Kind];if(Kind==0)Volume*=D->bSprinting?1.35f:.80f;
    if(D->Species==3)Volume*=.55f;
    const float Pitch=Variation.FRandRange(.965f,1.035f)*(D->Species==3?1.15f:D->Species==7?1.12f:1.f);
    Voice->SetPitchMultiplier(Pitch);Voice->SetVolumeMultiplier(Volume);
    Voices.Add(Voice);Kinds.Add(Kind);Ends.Add(GetWorld()->GetTimeSeconds()+Wave->Duration/Pitch+.10f);Voice->Play();
    switch(Kind)
    {
        case 0:++Steps;break;case 1:++QuickSounds;break;case 2:++HeavySounds;break;
        case 3:++Impacts;ImpactCooldown=.12f;break;case 4:++Deaths;break;
        case 5:++Charges;break;case 6:++Hurts;HurtCooldown=.25f;break;
        case 7:++SprintBreaths;break;case 8:++InjuredBreaths;break;
    }
}
void UDinoAudioComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return;
    for(int32 I=Voices.Num()-1;I>=0;--I)if(GetWorld()->GetTimeSeconds()>=Ends[I])RemoveVoice(I);
    StepCooldown=FMath::Max(0.f,StepCooldown-Dt);ImpactCooldown=FMath::Max(0.f,ImpactCooldown-Dt);
    HurtCooldown=FMath::Max(0.f,HurtCooldown-Dt);BreathCooldown=FMath::Max(0.f,BreathCooldown-Dt);
    if(!D->Combat->bCharging)StopKind(5);
    const FVector P=D->GetActorLocation();const float Travel=FVector::Dist2D(P,Previous);Previous=P;
    if(D->bDead)return;
    const bool Injured=D->Health->Fraction()<.50f;
    if(!D->bSprinting)StopKind(7);
    if(!Injured)StopKind(8);
    if(BreathCooldown<=0&&!D->Combat->bCharging&&!D->Combat->IsBusy()&&!D->bSwimming)
    {
        if(Injured){PlayEvent(8);BreathCooldown=Variation.FRandRange(2.8f,4.0f);}
        else if(D->bSprinting){PlayEvent(7);BreathCooldown=Variation.FRandRange(1.45f,2.1f);}
    }
    if(D->bInWater||!D->GetCharacterMovement()->IsMovingOnGround()||Travel>500||D->GetVelocity().Size2D()<50){Distance=0;return;}
    Distance+=Travel;const float Stride=D->Species==1?370:D->Species==6?340:D->Species==2?420:D->Species==4?390:D->Species==5?700:650;
    if(Distance>=Stride&&StepCooldown<=0){Distance=0;StepCooldown=D->Species==1?.13f:.22f;PlayEvent(0);}
}
