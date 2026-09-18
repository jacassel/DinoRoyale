#include "CombatComponent.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "DinoEffects.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
UCombatComponent::UCombatComponent(){PrimaryComponentTick.bCanEverTick=true;}
ADinosaurCharacter* UCombatComponent::Dino() const {return Cast<ADinosaurCharacter>(GetOwner());}
void UCombatComponent::Cancel(){bBracing=false;bCharging=false;bHitPending=false;RecoveryLeft=0;ChargeElapsed=0;}
bool UCombatComponent::SetBrace(bool Active)
{
    auto* D=Dino(); if(!D||D->bDead) return false;
    if(Active&&D->GetCharacterMovement()->IsFalling()) return false;
    bBracing=Active;
    if(Active){bCharging=false;bHitPending=false;D->GetCharacterMovement()->StopMovementImmediately();D->ConsumeMovementInputVector();}
    return true;
}
bool UCombatComponent::QuickAttack(){auto* D=Dino();if(!D||D->bDead||bBracing||bCharging||IsBusy())return false;Execute(false,0);return true;}
bool UCombatComponent::StartCharge(){auto* D=Dino();if(!D||D->bDead||bBracing||IsBusy()||D->Health->Fraction()<.25f)return false;bCharging=true;ChargeElapsed=0;return true;}
float UCombatComponent::ChargeFraction() const {auto* D=Dino();return D?FMath::Clamp(ChargeElapsed/FMath::Max(.1f,D->Stats().ChargeTime),0.f,1.f):0;}
bool UCombatComponent::ReleaseCharge(){if(!bCharging)return false;float Power=ChargeFraction();bCharging=false;auto* D=Dino();if(!D||D->bDead||D->Health->Fraction()<.25f)return false;Execute(true,Power);return true;}
void UCombatComponent::Execute(bool Charged,float Power)
{
    auto* D=Dino();const auto& S=D->Stats(); bChargedAttack=Charged;
    ++AttackSerial;
    AttackDuration=(Charged?S.ChargeRecovery:S.Recovery)/D->Health->AttackSpeedFactor();
    RecoveryLeft=AttackDuration; AttackElapsed=0; HitTime=S.Windup/D->Health->AttackSpeedFactor();
    PendingDamage=S.Damage*(Charged?FMath::Lerp(1.25f,S.ChargeMultiplier,Power):1.f);
    bHitPending=true;LastDealtDamage=0;
    if(Charged)
    {
        if(D->bSwimming)D->GetCharacterMovement()->Velocity=D->GetActorForwardVector()*S.LungeSpeed*.45f;
        else D->LaunchCharacter(D->GetActorForwardVector()*S.LungeSpeed*(.45f+.55f*Power)+FVector(0,0,D->Species==1?350:30),true,false);
    }
}
void UCombatComponent::DetectHits()
{
    auto* D=Dino();const auto& S=D->Stats();FVector Origin=D->GetActorLocation();
    // A single timed capsule sweep; an actor is considered once regardless of component count.
    FVector End=Origin+D->GetActorForwardVector()*S.AttackRange;
    TArray<FHitResult> Hits;FCollisionQueryParams Params(SCENE_QUERY_STAT(DinoAttack),false,D);
    GetWorld()->SweepMultiByObjectType(Hits,Origin,End,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeSphere(S.AttackWidth),Params);
    TSet<ADinosaurCharacter*> Seen;
    for(const auto& H:Hits)
    {
        auto* Target=Cast<ADinosaurCharacter>(H.GetActor());
        if(!Target||Seen.Contains(Target)||!D->IsEnemy(Target))continue;
        Seen.Add(Target);
        if(FVector::DotProduct(D->GetActorForwardVector(),(Target->GetActorLocation()-Origin).GetSafeNormal2D())<.15f)continue;
        FHitResult Wall; FCollisionQueryParams WallParams(SCENE_QUERY_STAT(DinoAttackWall),false,D); WallParams.AddIgnoredActor(Target);
        if(GetWorld()->LineTraceSingleByChannel(Wall,Origin,Target->GetActorLocation(),ECC_Visibility,WallParams))continue;
        float Before=Target->Health->Current;Target->ReceiveHit(PendingDamage,D);float Applied=Before-Target->Health->Current;LastDealtDamage+=Applied;++TotalHits;
        if(Applied>0)ADinoEffects::EmitBlood(GetWorld(),H.ImpactPoint.IsNearlyZero()?Target->GetActorLocation():FVector(H.ImpactPoint),D->GetActorForwardVector(),Applied);
    }
}
void UCombatComponent::TickComponent(float Dt,ELevelTick T,FActorComponentTickFunction* F)
{
    Super::TickComponent(Dt,T,F);
    if(bCharging)ChargeElapsed+=Dt;
    if(RecoveryLeft>0){RecoveryLeft=FMath::Max(0.f,RecoveryLeft-Dt);AttackElapsed+=Dt;}
    if(bHitPending&&AttackElapsed>=HitTime){bHitPending=false;DetectHits();}
}
