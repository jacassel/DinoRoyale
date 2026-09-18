#include "FoodSystem.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/ConfigCacheIni.h"
AFoodPlant::AFoodPlant()
{
    PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.25f;
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EdibleCycad"));SetRootComponent(Visual);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);Visual->SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Fern(TEXT("/Game/World/SM_Fern.SM_Fern"));
    if(Fern.Succeeded())Visual->SetStaticMesh(Fern.Object);
    Visual->SetRelativeScale3D(FVector(3));
}
void AFoodPlant::Consume(float Amount)
{
    Nutrition=FMath::Max(0.f,Nutrition-Amount);
    if(Nutrition<=0){RegrowTimer=45;Visual->SetRelativeScale3D(FVector(1.1f));}
}
void AFoodPlant::Tick(float Dt)
{
    Super::Tick(Dt);if(Nutrition<=0){RegrowTimer-=Dt;if(RegrowTimer<=0){Nutrition=1;Visual->SetRelativeScale3D(FVector(3));}}
}
UFoodInteractionComponent::UFoodInteractionComponent(){PrimaryComponentTick.bCanEverTick=true;}
AActor* UFoodInteractionComponent::FindFood(float Range) const
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||D->bDead)return nullptr;
    AActor* Best=nullptr;float BestDist=Range*Range;
    if(D->Species==2)
    {
        for(TActorIterator<AFoodPlant> It(GetWorld());It;++It)
        {
            float Dist=FVector::DistSquared2D(It->GetActorLocation(),D->GetActorLocation());
            if(It->IsAvailable()&&Dist<BestDist){Best=*It;BestDist=Dist;}
        }
    }
    else if(D->Species!=3)
    {
        for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
        {
            float Dist=FVector::DistSquared2D(It->GetActorLocation(),D->GetActorLocation());
            if(It->bDead&&It->Nutrition>0&&Dist<BestDist){Best=*It;BestDist=Dist;}
        }
    }
    return Best;
}
bool UFoodInteractionComponent::StartEating()
{
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||D->bDead||D->Combat->bBracing||D->Combat->IsBusy()||D->Combat->bCharging||D->GetCharacterMovement()->IsFalling())return false;
    Source=FindFood(D->Stats().AttackRange+260);bEating=Source.IsValid();return bEating;
}
void UFoodInteractionComponent::StopEating(){bEating=false;Source=nullptr;}
void UFoodInteractionComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);if(!bEating)return;
    auto* D=Cast<ADinosaurCharacter>(GetOwner());AActor* A=Source.Get();
    if(!D||D->bDead||!A||D->Combat->bBracing||D->Combat->IsBusy()||FVector::Dist2D(D->GetActorLocation(),A->GetActorLocation())>D->Stats().AttackRange+300){StopEating();return;}
    bool Available=false;
    if(auto* Plant=Cast<AFoodPlant>(A)){Available=Plant->IsAvailable();if(Available)Plant->Consume(Dt*.14f);}
    if(auto* Corpse=Cast<ADinosaurCharacter>(A)){Available=Corpse->bDead&&Corpse->Nutrition>0;if(Available)Corpse->Nutrition=FMath::Max(0.f,Corpse->Nutrition-Dt*.16f);}
    if(!Available){StopEating();return;}
    D->GetCharacterMovement()->StopMovementImmediately();D->Health->Heal(D->Health->Maximum*EatRate*Dt);FoodConsumed+=Dt;
    if(D->Health->Fraction()>=1)StopEating();
}
