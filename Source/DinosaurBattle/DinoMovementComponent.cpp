#include "DinoMovementComponent.h"
#include "DinosaurCharacter.h"
#include "CombatComponent.h"
#include "LostValleyWorld.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "FoodSystem.h"

namespace
{
class FSavedDinoMove : public FSavedMove_Character
{
public:
    using Super=FSavedMove_Character;
    bool Sprint=false;
    virtual void Clear() override {Super::Clear();Sprint=false;}
    virtual uint8 GetCompressedFlags() const override {return Super::GetCompressedFlags()|(Sprint?FLAG_Custom_0:0);}
    virtual bool CanCombineWith(const FSavedMovePtr& Other,ACharacter* C,float MaxDelta) const override
    {return Sprint==static_cast<const FSavedDinoMove*>(Other.Get())->Sprint&&Super::CanCombineWith(Other,C,MaxDelta);}
    virtual void SetMoveFor(ACharacter* C,float Dt,const FVector& A,FNetworkPredictionData_Client_Character& Data) override
    {Super::SetMoveFor(C,Dt,A,Data);Sprint=CastChecked<ADinosaurCharacter>(C)->bSprintRequested;}
    virtual void PrepMoveFor(ACharacter* C) override
    {Super::PrepMoveFor(C);CastChecked<ADinosaurCharacter>(C)->bSprintRequested=Sprint;}
};
class FDinoPredictionData : public FNetworkPredictionData_Client_Character
{
public:
    explicit FDinoPredictionData(const UCharacterMovementComponent& M):FNetworkPredictionData_Client_Character(M){}
    virtual FSavedMovePtr AllocateNewMove() override {return FSavedMovePtr(new FSavedDinoMove());}
};
}
void UDinoMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{Super::UpdateFromCompressedFlags(Flags);if(auto* D=Cast<ADinosaurCharacter>(CharacterOwner))D->bSprintRequested=(Flags&FSavedMove_Character::FLAG_Custom_0)!=0;}
FNetworkPredictionData_Client* UDinoMovementComponent::GetPredictionData_Client() const
{
    if(!ClientPredictionData)const_cast<UDinoMovementComponent*>(this)->ClientPredictionData=new FDinoPredictionData(*this);
    return ClientPredictionData;
}

float UDinoMovementComponent::GetMaxSpeed() const
{
    const auto* D=Cast<ADinosaurCharacter>(CharacterOwner);
    if(D&&D->GetNetMode()!=NM_Standalone)
    {
        if(D->bDead||D->Combat->bBracing||D->Food->bEating)return 0;
        if(MovementMode==MOVE_Walking||MovementMode==MOVE_Falling)
        {
            const bool Sprint=D->bSprintRequested&&!D->Stamina->bExhausted&&D->Stamina->Current>0&&!D->bInWater&&!D->Combat->bCharging&&!D->Combat->IsBusy()&&!IsFalling();
            const float Commit=D->Combat->IsBusy()&&D->Combat->bChargedAttack?D->Stats().HeavyMoveFactor:1.f;
            return D->Stats().Speed*D->Health->MovementFactor()*(D->Combat->bCharging?.55f:Commit)*(Sprint?D->Stats().SprintMultiplier:1.f)*(D->bInWater?D->WaterSpeedMultiplier:1.f);
        }
    }
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
