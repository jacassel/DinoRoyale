#include "DinoAnimationComponent.h"
#include "DinosaurCharacter.h"
#include "CombatComponent.h"
#include "HealthComponent.h"
#include "FoodSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
UDinoAnimationComponent::UDinoAnimationComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UDinoAnimationComponent::LoadSpecies()
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return;
    FString Name=D->Stats().AssetName;
    auto* Sk=LoadObject<USkeletalMesh>(nullptr,*(TEXT("/Game/Dinosaurs/")+Name+TEXT("/")+Name+TEXT(".")+Name));
    D->Placeholder->SetVisibility(!Sk);
    if(!Sk)return;
    // Mesh swaps retain overrides in Unreal; discard the prior species material instances.
    D->GetMesh()->EmptyOverrideMaterials();
    D->GetMesh()->SetSkeletalMesh(Sk);
    for(int32 I=0;I<D->GetMesh()->GetNumMaterials();++I)if(auto* MI=D->GetMesh()->CreateAndSetMaterialInstanceDynamic(I))MI->SetVectorParameterValue(TEXT("BaseTint"),D->Stats().Color);
    D->GetMesh()->SetRelativeLocation(FVector(0,0,-D->Stats().HalfHeight));
    D->GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);
    // Species setup happens after PostInitializeComponents. Smoothing must use
    // this anatomical mesh origin, rather than the constructor's zero offset.
    D->CacheInitialMeshOffset(D->GetMesh()->GetRelativeLocation(),D->GetMesh()->GetRelativeRotation());
    D->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    D->GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    Clips.Empty();
    for(const TCHAR* N:{TEXT("Idle"),TEXT("PivotLeft"),TEXT("PivotRight"),TEXT("Walk"),TEXT("Run"),TEXT("Swim"),TEXT("Quick"),TEXT("Charge"),TEXT("Heavy"),TEXT("Jump"),TEXT("Brace"),TEXT("Death"),TEXT("Eat")})
    {
        FString A=Name+TEXT("_")+N;
        if(auto* Clip=LoadObject<UAnimSequence>(nullptr,*(TEXT("/Game/Dinosaurs/")+Name+TEXT("/")+A+TEXT(".")+A))) Clips.Add(N,Clip);
    }
    State=TEXT("");LastAttackSerial=-1;Play(TEXT("Idle"),true);
}
void UDinoAnimationComponent::Play(const FString& Clip,bool Loop,float Rate,bool Restart)
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return;
    auto** Found=Clips.Find(Clip);if(!Found)return;
    if(State!=Clip||Restart){D->GetMesh()->PlayAnimation(*Found,Loop);State=Clip;}
    D->GetMesh()->SetPlayRate(Rate);
}
void UDinoAnimationComponent::TickComponent(float Dt,ELevelTick T,FActorComponentTickFunction* F)
{
    Super::TickComponent(Dt,T,F);auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return;
    if(D->bDead){Play(TEXT("Death"),false);return;}
    if(D->Combat->bBracing){Play(TEXT("Brace"),true);return;}
    if(D->Combat->bCharging){Play(TEXT("Charge"),true);return;}
    if(D->Food->bEating){Play(TEXT("Eat"),true);return;}
    if(D->Combat->IsBusy())
    {
        FString N=D->Combat->bChargedAttack?TEXT("Heavy"):TEXT("Quick");
        float Length=Clips.Contains(N)?Clips[N]->GetPlayLength():1;
        Play(N,false,Length/FMath::Max(.1f,D->Combat->AttackDuration),LastAttackSerial!=D->Combat->AttackSerial);
        LastAttackSerial=D->Combat->AttackSerial;return;
    }
    if(D->bSwimming){Play(TEXT("Swim"),true,.75f+.5f*D->GetVelocity().Size2D()/FMath::Max(1.f,D->GetCharacterMovement()->MaxSwimSpeed));return;}
    if(D->GetCharacterMovement()->IsFalling()){Play(TEXT("Jump"),false);return;}
    if(D->PivotVisual!=0){Play(D->PivotVisual<0?TEXT("PivotLeft"):TEXT("PivotRight"),true);return;}
    float Speed=D->GetVelocity().Size2D();
    if(Speed<35){Play(TEXT("Idle"),true);return;}
    const float GaitSpeed=D->Species==1?1000:D->Species==2?540:D->Species==3?800:680;
    Play(Speed>D->Stats().Speed*.45f?TEXT("Run"):TEXT("Walk"),true,FMath::Clamp(Speed/GaitSpeed,.4f,2.1f));
}
