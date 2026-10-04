#include "DinoMovementComponent.h"
#include "DinoGameState.h"
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
    int8 Pivot=0;
    virtual void Clear() override {Super::Clear();Sprint=false;Pivot=0;}
    virtual uint8 GetCompressedFlags() const override {return Super::GetCompressedFlags()|(Sprint?FLAG_Custom_0:0)|(Pivot<0?FLAG_Custom_1:0)|(Pivot>0?FLAG_Custom_2:0);}
    virtual bool CanCombineWith(const FSavedMovePtr& Other,ACharacter* C,float MaxDelta) const override
    {const auto* Move=static_cast<const FSavedDinoMove*>(Other.Get());return Sprint==Move->Sprint&&Pivot==Move->Pivot&&Super::CanCombineWith(Other,C,MaxDelta);}
    virtual void SetMoveFor(ACharacter* C,float Dt,const FVector& A,FNetworkPredictionData_Client_Character& Data) override
    {Super::SetMoveFor(C,Dt,A,Data);const auto* D=CastChecked<ADinosaurCharacter>(C);Sprint=D->bSprintRequested;Pivot=D->PivotInput;}
    virtual void PrepMoveFor(ACharacter* C) override
    {Super::PrepMoveFor(C);auto* D=CastChecked<ADinosaurCharacter>(C);D->bSprintRequested=Sprint;D->PivotInput=Pivot;}
};
class FDinoPredictionData : public FNetworkPredictionData_Client_Character
{
public:
    explicit FDinoPredictionData(const UCharacterMovementComponent& M):FNetworkPredictionData_Client_Character(M){}
    virtual FSavedMovePtr AllocateNewMove() override {return FSavedMovePtr(new FSavedDinoMove());}
};
}
void UDinoMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{Super::UpdateFromCompressedFlags(Flags);if(auto* D=Cast<ADinosaurCharacter>(CharacterOwner)){D->bSprintRequested=(Flags&FSavedMove_Character::FLAG_Custom_0)!=0;D->PivotInput=((Flags&FSavedMove_Character::FLAG_Custom_2)?1:0)-((Flags&FSavedMove_Character::FLAG_Custom_1)?1:0);}}
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
        if(D->MatchFrozen()||D->bDead||D->Combat->bBracing||D->Food->bEating)return 0;
        if(MovementMode==MOVE_Walking||MovementMode==MOVE_Falling)
        {
            const bool Sprint=D->bSprintRequested&&!D->Stamina->bExhausted&&D->Stamina->Current>0&&!D->bInWater&&!D->Combat->bCharging&&!D->Combat->IsBusy()&&!IsFalling();
            const float Commit=D->Combat->IsBusy()&&D->Combat->bChargedAttack?D->Stats().HeavyMoveFactor:1.f;
            return D->Stats().Speed*D->Health->MovementFactor()*(D->Combat->bCharging?.55f:Commit)*(Sprint?D->Stats().SprintMultiplier:1.f)*(D->bInWater?D->WaterSpeedMultiplier:1.f);
        }
    }
    return MovementMode==MOVE_Custom?MaxSwimSpeed:Super::GetMaxSpeed();
}
void UDinoMovementComponent::PhysicsRotation(float Dt)
{
    auto* D=Cast<ADinosaurCharacter>(CharacterOwner);
    if(!D){Super::PhysicsRotation(Dt);return;}
    const bool Pivoting=D->PivotInput!=0&&D->CanPivot()&&Acceleration.IsNearlyZero();
    D->PivotVisual=Pivoting?D->PivotInput:0;
    if(Pivoting)
    {
        // PhysicsRotation runs inside predicted movement on the owning client and server.
        // The saved input is replayed after corrections; control rotation remains untouched.
        const float Commitment=D->Combat->IsBusy()?(D->Combat->bChargedAttack?D->Stats().HeavyTurnFactor:.55f):D->Combat->bCharging?.45f:1.f;
        const float Step=D->PivotInput*D->Stats().PivotRate*D->Health->MovementFactor()*Commitment*Dt;
        FRotator Rotation=UpdatedComponent->GetComponentRotation();Rotation.Yaw+=Step;
        MoveUpdatedComponent(FVector::ZeroVector,Rotation,true);
        return;
    }
    Super::PhysicsRotation(Dt);
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
