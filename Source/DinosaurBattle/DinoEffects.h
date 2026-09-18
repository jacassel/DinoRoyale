#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinoEffects.generated.h"
class UInstancedStaticMeshComponent;
struct FBloodParticle
{
    FVector Position,Velocity;
    float Life=0,MaxLife=0,Radius=5;
    bool bGrounded=false;
};
/** Bounded, optional, local visual effects. They never participate in collision or damage. */
UCLASS()
class DINOSAURBATTLE_API ADinoEffects : public AActor
{
    GENERATED_BODY()
public:
    ADinoEffects();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    UPROPERTY(VisibleAnywhere) UInstancedStaticMeshComponent* BloodMesh;
    static void EmitBlood(UWorld* World,const FVector& Position,const FVector& Direction,float Damage);
    static void ClearBlood(UWorld* World);
    int32 TotalEmitted=0;
    int32 ActiveCount() const{return Particles.Num();}
private:
    TArray<FBloodParticle> Particles;
    void Burst(const FVector& P,const FVector& Direction,float Damage);
};
