#include "DinoEffects.h"
#include "DinoPlayerController.h"
#include "LostValleyWorld.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
ADinoEffects::ADinoEffects()
{
    PrimaryActorTick.bCanEverTick=true;BloodMesh=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("OptionalBlood"));SetRootComponent(BloodMesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));BloodMesh->SetStaticMesh(Sphere.Object);
    BloodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);BloodMesh->SetCastShadow(false);BloodMesh->SetCanEverAffectNavigation(false);
}
void ADinoEffects::BeginPlay(){Super::BeginPlay();BloodMesh->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_Blood.M_Blood")));}
void ADinoEffects::EmitBlood(UWorld* World,const FVector& Position,const FVector& Direction,float Damage)
{
    if(!World)return;auto* PC=Cast<ADinoPlayerController>(World->GetFirstPlayerController());if(!PC||!PC->bBloodEnabled||!PC->GetPawn())return;
    if(FVector::DistSquared(Position,PC->GetPawn()->GetActorLocation())>FMath::Square(8500.f))return;
    for(TActorIterator<ADinoEffects> It(World);It;++It){It->Burst(Position,Direction,Damage);return;}
}
void ADinoEffects::ClearBlood(UWorld* World){if(World)for(TActorIterator<ADinoEffects> It(World);It;++It){It->Particles.Empty();It->BloodMesh->ClearInstances();}}
void ADinoEffects::Burst(const FVector& P,const FVector& Direction,float Damage)
{
    int32 Count=FMath::Clamp(FMath::RoundToInt(Damage/22),6,20);
    for(int32 I=0;I<Count;++I)
    {
        if(Particles.Num()>=200)Particles.RemoveAtSwap(0,1,EAllowShrinking::No);
        FBloodParticle V;V.Position=P+FMath::VRand()*12;V.Velocity=Direction*FMath::FRandRange(100.f,360.f)+FMath::VRand()*FMath::FRandRange(100.f,260.f)+FVector(0,0,FMath::FRandRange(80.f,250.f));
        V.Life=V.MaxLife=FMath::FRandRange(.6f,1.4f);V.Radius=FMath::FRandRange(4.f,9.f);Particles.Add(V);++TotalEmitted;
    }
}
void ADinoEffects::Tick(float Dt)
{
    Super::Tick(Dt);auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController());
    if(!PC||!PC->bBloodEnabled){if(!Particles.IsEmpty())ClearBlood(GetWorld());return;}
    BloodMesh->ClearInstances();
    for(int32 I=Particles.Num()-1;I>=0;--I)
    {
        auto& P=Particles[I];P.Life-=Dt;if(P.Life<=0){Particles.RemoveAtSwap(I,1,EAllowShrinking::No);continue;}
        if(!P.bGrounded)
        {
            P.Velocity.Z-=980*Dt;P.Position+=P.Velocity*Dt;float Floor=ALostValleyWorld::HeightAt(P.Position.X,P.Position.Y)+3;
            if(P.Position.Z<Floor){P.Position.Z=Floor;P.bGrounded=true;P.Radius*=1.7f;}
        }
        float Scale=P.Radius/50.f*FMath::Clamp(P.Life/.25f,0.f,1.f);
        BloodMesh->AddInstance(FTransform(FRotator::ZeroRotator,P.Position,FVector(Scale,Scale,Scale*(P.bGrounded?.12f:1.f))));
    }
}
