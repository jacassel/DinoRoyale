#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "FoodSystem.h"
#include "DinoEffects.h"
#include "DinoAudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/SkeletalMeshComponent.h"
UCombatComponent::UCombatComponent(){PrimaryComponentTick.bCanEverTick=true;SetIsReplicatedByDefault(true);}
ADinosaurCharacter* UCombatComponent::Dino() const {return Cast<ADinosaurCharacter>(GetOwner());}
void UCombatComponent::Cancel(){if(!GetOwner()->HasAuthority()){return;}bBracing=false;bCharging=false;bHitPending=false;RecoveryLeft=0;ChargeElapsed=0;ComboCount=0;ComboResetLeft=0;BufferedQuick=0;HitActors.Empty();}
bool UCombatComponent::SetBrace(bool Active)
{if(!GetOwner()->HasAuthority()){return false;}
    auto* D=Dino(); if(!D||D->bDead||(Active&&D->MatchFrozen())) return false;
    if(Active&&(D->GetCharacterMovement()->IsFalling()||IsBusy()||D->Stamina->bExhausted||D->Stamina->Current<=0)) return false;
    bBracing=Active;
    if(Active){bCharging=false;bHitPending=false;D->GetCharacterMovement()->StopMovementImmediately();D->ConsumeMovementInputVector();}
    return true;
}
bool UCombatComponent::QuickAttack()
{if(!GetOwner()->HasAuthority()){return false;}
    auto* D=Dino();if(!D||D->bDead||D->MatchFrozen()||bBracing||bCharging)return false;
    if(IsBusy()){if(RecoveryLeft<=.18f)BufferedQuick=.2f;return false;}
    D->Food->StopEating();
    bWeakAttack=!D->Stamina->CanSpend(D->Stats().QuickCost);
    // Even exhausted animals retain a weaker bite. Repeated empty bites cannot reset regeneration.
    if(!D->Stamina->bExhausted&&D->Stamina->Current>0)D->Stamina->Drain(FMath::Min(D->Stamina->Current,D->Stats().QuickCost));
    if(ComboResetLeft<=0||ComboCount>=3)ComboCount=0;
    ++ComboCount;Execute(false,0);return true;
}
bool UCombatComponent::StartCharge()
{if(!GetOwner()->HasAuthority()){return false;}
    auto* D=Dino();if(!D||D->bDead||D->MatchFrozen()||bBracing||bCharging||IsBusy()||D->Health->Fraction()<.25f||!D->Stamina->CanSpend(D->Stats().HeavyCost))return false;
    D->Food->StopEating();D->RevealNoise();bCharging=true;ChargeElapsed=0;D->PlayCombatSound(5);return true;
}
float UCombatComponent::ChargeFraction() const {auto* D=Dino();return D?FMath::Clamp(ChargeElapsed/FMath::Max(.1f,D->Stats().ChargeTime),0.f,1.f):0;}
bool UCombatComponent::ReleaseCharge(){if(!GetOwner()->HasAuthority()){return false;}if(!bCharging)return false;float Power=ChargeFraction();bCharging=false;auto* D=Dino();if(!D||D->bDead||D->Health->Fraction()<.25f||!D->Stamina->Spend(D->Stats().HeavyCost))return false;Execute(true,Power);return true;}
void UCombatComponent::Execute(bool Charged,float Power)
{if(!GetOwner()->HasAuthority()){return;}
    auto* D=Dino();const auto& S=D->Stats(); bChargedAttack=Charged;
    ++AttackSerial;D->RevealNoise();
    D->PlayCombatSound(Charged?2:1);
    AttackDuration=(Charged?S.ChargeRecovery:S.Recovery)/D->Health->AttackSpeedFactor();
    if(!Charged&&bWeakAttack)AttackDuration*=S.WeakAttackRecovery;
    RecoveryLeft=AttackDuration+(!Charged&&ComboCount==3?S.ComboRecovery/D->Health->AttackSpeedFactor():0);
    ComboResetLeft=RecoveryLeft+S.ComboReset;BufferedQuick=0;
    AttackElapsed=0; HitTime=(Charged?S.HeavyWindup:S.Windup)/D->Health->AttackSpeedFactor();
    CurrentHeavyPower=Power;CommitDirection=D->GetActorForwardVector();HitActors.Empty();
    PendingDamage=S.Damage*(Charged?FMath::Lerp(1.15f,S.ChargeMultiplier,Power):(bWeakAttack?S.WeakAttackDamage:ComboCount==3?1.12f:1.f));
    bHitPending=true;LastDealtDamage=0;bHasContact=false;
    if(Charged&&D->Species==1&&!D->bSwimming)D->LaunchCharacter(CommitDirection*S.LungeSpeed*(.55f+.45f*Power)+FVector(0,0,260),true,false);
}
void UCombatComponent::DetectHits()
{if(!GetOwner()->HasAuthority()){return;}
    auto* D=Dino();const auto& S=D->Stats();FVector Origin=D->GetActorLocation();
    // Timed sweep window; each target is damaged once across all samples/components.
    FVector End=Origin+D->GetActorForwardVector()*S.AttackRange*(bChargedAttack?S.HeavyReach:1.f);
    TArray<FHitResult> Hits;FCollisionQueryParams Params(SCENE_QUERY_STAT(DinoAttack),false,D);
    if(D->Species>=4)
    {
        // Follow the animated striking anatomy; never sweep the full body capsule.
        FName Socket=D->Species==4?(ComboCount==3&&!bChargedAttack?TEXT("spine"):TEXT("club_tip")):
            D->Species==5?(!bChargedAttack&&ComboCount==2?TEXT("tail_04"):ComboCount==3?TEXT("stomp_r"):TEXT("stomp_l")):TEXT("head_impact");
        FVector Contact=D->GetMesh()->GetSocketLocation(Socket);
        const float Width=D->Species==4?(Socket==TEXT("spine")?155.f:80.f):D->Species==5?(Socket==TEXT("tail_04")?95.f:bChargedAttack?210.f:115.f):55.f;
        if(Socket==TEXT("spine"))Contact+=D->GetActorRightVector()*115;
        if(D->Species==5&&bChargedAttack)Contact=(Contact+D->GetMesh()->GetSocketLocation(TEXT("stomp_r")))*.5f;
        const FVector Start=bHasContact?PreviousContact:Contact;PreviousContact=Contact;bHasContact=true;
        GetWorld()->SweepMultiByObjectType(Hits,Start,Contact,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeSphere(Width),Params);
    }
    else GetWorld()->SweepMultiByObjectType(Hits,Origin,End,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeSphere(S.AttackWidth),Params);
    for(const auto& H:Hits)
    {
        auto* Target=Cast<ADinosaurCharacter>(H.GetActor());
        if(!Target||HitActors.Contains(Target)||!D->IsEnemy(Target))continue;
        if(D->Species<4&&FVector::DotProduct(D->GetActorForwardVector(),(Target->GetActorLocation()-Origin).GetSafeNormal2D())<.15f)continue;
        FHitResult Wall; FCollisionQueryParams WallParams(SCENE_QUERY_STAT(DinoAttackWall),false,D); WallParams.AddIgnoredActor(Target);
        if(GetWorld()->LineTraceSingleByChannel(Wall,Origin,Target->GetActorLocation(),ECC_Visibility,WallParams))continue;
        HitActors.Add(Target);
        float Before=Target->Health->Current;Target->ReceiveHit(PendingDamage,D);float Applied=Before-Target->Health->Current;LastDealtDamage+=Applied;++TotalHits;
        if(Applied>0)D->PlayCombatSound(3);
        if(Applied>0&&bChargedAttack&&!Target->Combat->bBracing&&!Target->bDead)
        {
            // A small pounce cannot repeatedly interrupt an apex animal's committed windup.
            const float MassRatio=FMath::Min(1.f,S.Radius/Target->Stats().Radius);
            if(MassRatio>=.65f)Target->Combat->bCharging=false;
            const FVector ImpactDirection=D->Species==4||D->Species==5?(Target->GetActorLocation()-Origin).GetSafeNormal2D():CommitDirection;
            Target->LaunchCharacter(ImpactDirection*S.HeavyKnockback*MassRatio*(.5f+.5f*CurrentHeavyPower)+FVector(0,0,40*MassRatio),true,false);
        }
        if(Applied>0)D->MulticastBlood(H.ImpactPoint.IsNearlyZero()?Target->GetActorLocation():FVector(H.ImpactPoint),D->GetActorForwardVector(),Applied);
    }
}
void UCombatComponent::TickComponent(float Dt,ELevelTick T,FActorComponentTickFunction* F)
{if(!GetOwner()->HasAuthority()){return;}
    Super::TickComponent(Dt,T,F);
    auto* D=Dino();if(!D||D->bDead||D->MatchFrozen())return;
    ComboResetLeft=FMath::Max(0.f,ComboResetLeft-Dt);
    if(bCharging){ChargeElapsed+=Dt;D->RevealNoise();}
    if(RecoveryLeft>0){RecoveryLeft=FMath::Max(0.f,RecoveryLeft-Dt);AttackElapsed+=Dt;}
    if(bChargedAttack&&IsBusy()&&AttackElapsed<D->Stats().HeavyDriveTime&&!bBracing)
    {
        auto* M=D->GetCharacterMovement();FVector V=CommitDirection*D->Stats().LungeSpeed*(.55f+.45f*CurrentHeavyPower)*(D->bSwimming?.45f:1.f);
        if(D->Species==6)V*=FMath::Clamp(AttackElapsed/.30f,.15f,1.f);
        M->Velocity.X=V.X;M->Velocity.Y=V.Y;
    }
    if(bHitPending&&AttackElapsed>=HitTime)
    {
        DetectHits();
        if((D->Species<4&&!bChargedAttack)||AttackElapsed>=HitTime+(D->Species>=4?.22f:.18f))
        {
            bHitPending=false;
            if(bChargedAttack&&HitActors.IsEmpty())RecoveryLeft+=D->Stats().HeavyMissRecovery;
        }
    }
    if(BufferedQuick>0){BufferedQuick-=Dt;if(!IsBusy()){BufferedQuick=0;QuickAttack();}}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCombatComponent,bBracing);
    DOREPLIFETIME(UCombatComponent,bCharging);
    DOREPLIFETIME(UCombatComponent,bChargedAttack);
    DOREPLIFETIME(UCombatComponent,ChargeElapsed);
    DOREPLIFETIME(UCombatComponent, RecoveryLeft);
    DOREPLIFETIME(UCombatComponent, AttackElapsed);
    DOREPLIFETIME(UCombatComponent, AttackDuration);
    DOREPLIFETIME(UCombatComponent,AttackSerial);
    DOREPLIFETIME(UCombatComponent,ComboCount);
    DOREPLIFETIME(UCombatComponent,bWeakAttack);
    DOREPLIFETIME(UCombatComponent,LastDealtDamage);
    DOREPLIFETIME(UCombatComponent,TotalHits);
}
