#include "HealthComponent.h"
#include "Engine/World.h"
UHealthComponent::UHealthComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UHealthComponent::Reset(float Max,float Delay,float Rate){Maximum=Max;Current=Max;RegenDelay=Delay;RegenRate=Rate;LastDamageTime=-100;}
float UHealthComponent::Receive(float Amount)
{
    if(IsDead()||bInvulnerable) return 0;
    float Applied=FMath::Clamp(Amount,0.f,Current); Current-=Applied;
    LastDamageTime=GetWorld()->GetTimeSeconds(); HitFlash=.24f; return Applied;
}
void UHealthComponent::Heal(float Amount){if(!IsDead()) Current=FMath::Clamp(Current+Amount,0.f,Maximum);}
void UHealthComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick); HitFlash=FMath::Max(0.f,HitFlash-Dt);
    if(!IsDead() && GetWorld()->GetTimeSeconds()-LastDamageTime>=RegenDelay) Heal(Maximum*RegenRate*Dt);
}
