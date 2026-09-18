#include "DinoMovementComponent.h"
#include "DinosaurCharacter.h"
#include "CombatComponent.h"
#include "LostValleyWorld.h"

float UDinoMovementComponent::GetMaxSpeed() const
{
    return MovementMode==MOVE_Custom?MaxSwimSpeed:Super::GetMaxSpeed();
}
void UDinoMovementComponent::PhysCustom(float Dt,int32 Iterations)
{
    auto* D=Cast<ADinosaurCharacter>(CharacterOwner);
    if(!D||!D->bSwimming||Dt<MIN_TICK_TIME){Super::PhysCustom(Dt,Iterations);return;}
    if(D->Combat->bBracing){Velocity=FVector::ZeroVector;return;}
    Acceleration.Z=0;Velocity.Z=0;
    CalcVelocity(Dt,2.5f,false,1200.f);
    const float TargetZ=D->WaterSurface-D->Stats().HalfHeight*.25f;
    Velocity.Z=FMath::Clamp((TargetZ-UpdatedComponent->GetComponentLocation().Z)*5.f,-300.f,300.f);
    const FVector Start=UpdatedComponent->GetComponentLocation();
    FHitResult Hit;
    SafeMoveUpdatedComponent(Velocity*Dt,UpdatedComponent->GetComponentQuat(),true,Hit);
    if(Hit.IsValidBlockingHit())
    {
        HandleImpact(Hit,Dt,Velocity*Dt);
        SlideAlongSurface(Velocity*Dt,1.f-Hit.Time,Hit.Normal,Hit,true);
    }
    Velocity=(UpdatedComponent->GetComponentLocation()-Start)/Dt;
}
