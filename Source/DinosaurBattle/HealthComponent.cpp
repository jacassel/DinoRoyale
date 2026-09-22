#include "Net/UnrealNetwork.h"
#include "HealthComponent.h"
#include "DinosaurCharacter.h"
#include "HungerComponent.h"
#include "Engine/World.h"
UHealthComponent::UHealthComponent(){PrimaryComponentTick.bCanEverTick=true;SetIsReplicatedByDefault(true);}
void UHealthComponent::Reset(float Max,float Delay,float Rate){if(!GetOwner()->HasAuthority()){return;}Maximum=Max;Current=Max;RegenDelay=Delay;RegenRate=Rate;LastDamageTime=-100;}
float UHealthComponent::Receive(float Amount)
{if(!GetOwner()->HasAuthority()){return 0;}
    if(IsDead()||bInvulnerable) return 0;
    float Applied=FMath::Clamp(Amount,0.f,Current); Current-=Applied;
    LastDamageTime=GetWorld()->GetTimeSeconds(); HitFlash=.24f; return Applied;
}
void UHealthComponent::Heal(float Amount){if(!GetOwner()->HasAuthority()){return;}if(!IsDead()) Current=FMath::Clamp(Current+Amount,0.f,Maximum);}
void UHealthComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{if(!GetOwner()->HasAuthority()){return;}
    Super::TickComponent(Dt,Type,Tick); HitFlash=FMath::Max(0.f,HitFlash-Dt);
    if(!IsDead() && GetWorld()->GetTimeSeconds()-LastDamageTime>=RegenDelay) {auto* D=Cast<ADinosaurCharacter>(GetOwner());Heal(Maximum*RegenRate*Dt*(D?D->Hunger->HealthRegenFactor():1.f));}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UHealthComponent,Current);
    DOREPLIFETIME(UHealthComponent,Maximum);
    DOREPLIFETIME(UHealthComponent,RegenDelay);
    DOREPLIFETIME(UHealthComponent, RegenRate);
    DOREPLIFETIME(UHealthComponent, LastDamageTime);
    DOREPLIFETIME(UHealthComponent, HitFlash);
}
