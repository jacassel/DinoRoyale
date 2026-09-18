#pragma once
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinoMovementComponent.generated.h"

/** Collision-aware surface swimming, shared by human and AI dinosaurs. */
UCLASS()
class DINOSAURBATTLE_API UDinoMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    virtual float GetMaxSpeed() const override;
protected:
    virtual void PhysCustom(float DeltaTime,int32 Iterations) override;
};
