#include "CombatComponent.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "FoodSystem.h"
#include "DinoEffects.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
UCombatComponent::UCombatComponent(){PrimaryComponentTick.bCanEverTick=true;}
ADinosaurCharacter* UCombatComponent::Dino() const {return Cast<ADinosaurCharacter>(GetOwner());}
void UCombatComponent::Cancel(){bBracing=false;bCharging=false;bHitPending=false;RecoveryLeft=0;ChargeElapsed=0;ComboCount=0;ComboResetLeft=0;BufferedQuick=0;HitActors.Empty();}
bool UCombatComponent::SetBrace(bool Active)
{
    auto* D=Dino(); if(!D||D->bDead) return false;
    if(Active&&(D->GetCharacterMovement()->IsFalling()||IsBusy()||D->Stamina->bExhausted||D->Stamina->Current<=0)) return false;
    bBracing=Active;
    if(Active){bCharging=false;bHitPending=false;D->GetCharacterMovement()->StopMovementImmediately();D->ConsumeMovementInputVector();}
    return true;
}
bool UCombatComponent::QuickAttack()
{
    auto* D=Dino();if(!D||D->bDead||bBracing||bCharging)return false;
    if(IsBusy()){if(RecoveryLeft<=.18f)BufferedQuick=.2f;return false;}
    D->Food->StopEating();
    bWeakAttack=!D->Stamina->CanSpend(D->Stats().QuickCost);
    // Even exhausted animals retain a weaker bite. Repeated empty bites cannot reset regeneration.
    if(!D->Stamina->bExhausted&&D->Stamina->Current>0)D->Stamina->Drain(FMath::Min(D->Stamina->Current,D->Stats().QuickCost));
    if(ComboResetLeft<=0||ComboCount>=3)ComboCount=0;
    ++ComboCount;Execute(false,0);return true;
}
bool UCombatComponent::StartCharge()
{
    auto* D=Dino();if(!D||D->bDead||bBracing||bCharging||IsBusy()||D->Health->Fraction()<.25f||!D->Stamina->CanSpend(D->Stats().HeavyCost))return false;
    D->Food->StopEating();bCharging=true;ChargeElapsed=0;return true;
}
float UCombatComponent::ChargeFraction() const {auto* D=Dino();return D?FMath::Clamp(ChargeElapsed/FMath::Max(.1f,D->Stats().ChargeTime),0.f,1.f):0;}
bool UCombatComponent::ReleaseCharge(){if(!bCharging)return false;float Power=ChargeFraction();bCharging=false;auto* D=Dino();if(!D||D->bDead||D->Health->Fraction()<.25f||!D->Stamina->Spend(D->Stats().HeavyCost))return false;Execute(true,Power);return true;}
void UCombatComponent::Execute(bool Charged,float Power)
{
    auto* D=Dino();const auto& S=D->Stats(); bChargedAttack=Charged;
    ++AttackSerial;
    AttackDuration=(Charged?S.ChargeRecovery:S.Recovery)/D->Health->AttackSpeedFactor();
    if(!Charged&&bWeakAttack)AttackDuration*=S.WeakAttackRecovery;
    RecoveryLeft=AttackDuration+(!Charged&&ComboCount==3?S.ComboRecovery/D->Health->AttackSpeedFactor():0);
    ComboResetLeft=RecoveryLeft+S.ComboReset;BufferedQuick=0;
    AttackElapsed=0; HitTime=(Charged?S.HeavyWindup:S.Windup)/D->Health->AttackSpeedFactor();
    CurrentHeavyPower=Power;CommitDirection=D->GetActorForwardVector();HitActors.Empty();
    PendingDamage=S.Damage*(Charged?FMath::Lerp(1.15f,S.ChargeMultiplier,Power):(bWeakAttack?S.WeakAttackDamage:ComboCount==3?1.12f:1.f));
    bHitPending=true;LastDealtDamage=0;
    if(Charged&&D->Species==1&&!D->bSwimming)D->LaunchCharacter(CommitDirection*S.LungeSpeed*(.55f+.45f*Power)+FVector(0,0,260),true,false);
}
void UCombatComponent::DetectHits()
{
    auto* D=Dino();const auto& S=D->Stats();FVector Origin=D->GetActorLocation();
    // Timed sweep window; each target is damaged once across all samples/components.
    FVector End=Origin+D->GetActorForwardVector()*S.AttackRange*(bChargedAttack?S.HeavyReach:1.f);
    TArray<FHitResult> Hits;FCollisionQueryParams Params(SCENE_QUERY_STAT(DinoAttack),false,D);
    GetWorld()->SweepMultiByObjectType(Hits,Origin,End,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeSphere(S.AttackWidth),Params);
    for(const auto& H:Hits)
    {
        auto* Target=Cast<ADinosaurCharacter>(H.GetActor());
        if(!Target||HitActors.Contains(Target)||!D->IsEnemy(Target))continue;
        if(FVector::DotProduct(D->GetActorForwardVector(),(Target->GetActorLocation()-Origin).GetSafeNormal2D())<.15f)continue;
        FHitResult Wall; FCollisionQueryParams WallParams(SCENE_QUERY_STAT(DinoAttackWall),false,D); WallParams.AddIgnoredActor(Target);
        if(GetWorld()->LineTraceSingleByChannel(Wall,Origin,Target->GetActorLocation(),ECC_Visibility,WallParams))continue;
        HitActors.Add(Target);
        float Before=Target->Health->Current;Target->ReceiveHit(PendingDamage,D);float Applied=Before-Target->Health->Current;LastDealtDamage+=Applied;++TotalHits;
        if(Applied>0&&bChargedAttack&&!Target->Combat->bBracing&&!Target->bDead)
        {
            Target->Combat->bCharging=false;
            Target->LaunchCharacter(CommitDirection*S.HeavyKnockback*(.5f+.5f*CurrentHeavyPower)+FVector(0,0,40),true,false);
        }
        if(Applied>0)ADinoEffects::EmitBlood(GetWorld(),H.ImpactPoint.IsNearlyZero()?Target->GetActorLocation():FVector(H.ImpactPoint),D->GetActorForwardVector(),Applied);
    }
}
void UCombatComponent::TickComponent(float Dt,ELevelTick T,FActorComponentTickFunction* F)
{
    Super::TickComponent(Dt,T,F);
    auto* D=Dino();if(!D||D->bDead)return;
    ComboResetLeft=FMath::Max(0.f,ComboResetLeft-Dt);
    if(bCharging)ChargeElapsed+=Dt;
    if(RecoveryLeft>0){RecoveryLeft=FMath::Max(0.f,RecoveryLeft-Dt);AttackElapsed+=Dt;}
    if(bChargedAttack&&IsBusy()&&AttackElapsed<D->Stats().HeavyDriveTime&&!bBracing)
    {
        auto* M=D->GetCharacterMovement();FVector V=CommitDirection*D->Stats().LungeSpeed*(.55f+.45f*CurrentHeavyPower)*(D->bSwimming?.45f:1.f);
        M->Velocity.X=V.X;M->Velocity.Y=V.Y;
    }
    if(bHitPending&&AttackElapsed>=HitTime)
    {
        DetectHits();
        if(!bChargedAttack||AttackElapsed>=HitTime+.18f)
        {
            bHitPending=false;
            if(bChargedAttack&&HitActors.IsEmpty())RecoveryLeft+=D->Stats().HeavyMissRecovery;
        }
    }
    if(BufferedQuick>0){BufferedQuick-=Dt;if(!IsBusy()){BufferedQuick=0;QuickAttack();}}
}
