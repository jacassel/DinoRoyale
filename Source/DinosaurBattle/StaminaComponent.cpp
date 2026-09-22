#include "Net/UnrealNetwork.h"
#include "StaminaComponent.h"
#include "DinosaurCharacter.h"
#include "CombatComponent.h"
#include "FoodSystem.h"
#include "HungerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
UStaminaComponent::UStaminaComponent(){PrimaryComponentTick.bCanEverTick=true;SetIsReplicatedByDefault(true);}
void UStaminaComponent::Reset(){if(!GetOwner()->HasAuthority()){return;}auto* D=Cast<ADinosaurCharacter>(GetOwner());Maximum=D?D->Stats().MaxStamina:100;Current=Maximum;bExhausted=false;ExhaustionLeft=RegenDelayLeft=0;}
bool UStaminaComponent::Spend(float Amount){if(!GetOwner()->HasAuthority()){return false;}if(!CanSpend(Amount))return false;Drain(Amount);return true;}
void UStaminaComponent::Drain(float Amount)
{if(!GetOwner()->HasAuthority()){return;}
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||Amount<=0)return;
    Current=FMath::Max(0.f,Current-Amount);RegenDelayLeft=D->Stats().StaminaRegenDelay;
    if(Current<=.01f&&!bExhausted){bExhausted=true;ExhaustionLeft=D->Stats().ExhaustionDuration;}
}
void UStaminaComponent::Restore(float Amount){if(!GetOwner()->HasAuthority()){return;}Current=FMath::Clamp(Current+Amount,0.f,Maximum);}
void UStaminaComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{if(!GetOwner()->HasAuthority()){return;}
    Super::TickComponent(Dt,Type,Tick);auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||D->bDead)return;
    const auto& S=D->Stats();ExhaustionLeft=FMath::Max(0.f,ExhaustionLeft-Dt);RegenDelayLeft=FMath::Max(0.f,RegenDelayLeft-Dt);
    if(D->bSprinting)Drain(S.SprintDrain*Dt);
    else if(D->Combat->bBracing){Drain(S.BraceDrain*Dt);if(bExhausted)D->Combat->SetBrace(false);}
    else if(RegenDelayLeft<=0&&!D->Combat->bCharging)
    {
        float Rate=D->GetVelocity().Size2D()<40?S.StaminaIdleRegen:S.StaminaWalkRegen;
        if(D->Combat->IsBusy()||D->GetCharacterMovement()->IsFalling())Rate*=S.StaminaBusyRegenFactor;
        Restore(Rate*Dt*D->Hunger->StaminaRegenFactor());
    }
    if(bExhausted&&ExhaustionLeft<=0&&Fraction()>=S.ExhaustionResume)bExhausted=false;
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UStaminaComponent,Current);
    DOREPLIFETIME(UStaminaComponent,Maximum);
    DOREPLIFETIME(UStaminaComponent,ExhaustionLeft);
    DOREPLIFETIME(UStaminaComponent,RegenDelayLeft);
    DOREPLIFETIME(UStaminaComponent,bExhausted);
}
