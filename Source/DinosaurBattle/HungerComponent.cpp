#include "HungerComponent.h"
#include "Net/UnrealNetwork.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "FoodSystem.h"
UHungerComponent::UHungerComponent(){PrimaryComponentTick.bCanEverTick=true;SetIsReplicatedByDefault(true);}
void UHungerComponent::Reset(){if(!GetOwner()->HasAuthority()){return;}auto* D=Cast<ADinosaurCharacter>(GetOwner());Maximum=D?D->Stats().MaxHunger:100;Current=Maximum;}
void UHungerComponent::Restore(float Amount){if(!GetOwner()->HasAuthority()){return;}Current=FMath::Clamp(Current+Amount,0.f,Maximum);}
float UHungerComponent::HealthRegenFactor() const
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return 1;
    const float F=Fraction();return F<.40f?0:F<.70f?D->Stats().HungryHealthRegen:F<.85f?1:D->Stats().FedHealthRegen;
}
float UHungerComponent::StaminaRegenFactor() const
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D)return 1;
    const float F=Fraction();return F<=.20f?0:F<.40f?D->Stats().VeryHungryStaminaRegen:F<.70f?D->Stats().HungryStaminaRegen:F<.85f?1:D->Stats().FedStaminaRegen;
}
void UHungerComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{if(!GetOwner()->HasAuthority()){return;}
    Super::TickComponent(Dt,Type,Tick);auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||D->bDead)return;
    if(!D->Food->bEating)Current=FMath::Max(0.f,Current-Dt*(D->Stats().HungerDrain+(D->bSprinting?D->Stats().HungerSprintDrain:0)));
    // Starvation is not a combat hit: it must never block the safe-feeding window.
    if(Fraction()<=.10f&&!D->Food->bEating&&!D->Health->bInvulnerable)
    {
        D->Health->Current=FMath::Max(0.f,D->Health->Current-D->Health->Maximum*D->Stats().StarvationRate*Dt);
        if(D->Health->IsDead()){D->LastAttacker=nullptr;D->DamageContributors.Empty();D->Die();}
    }
}

void UHungerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UHungerComponent,Current);
    DOREPLIFETIME(UHungerComponent,Maximum);
}
