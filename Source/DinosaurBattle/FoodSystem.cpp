#include "FoodSystem.h"
#include "Net/UnrealNetwork.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "HungerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "LostValleyWorld.h"
#include "CombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/PlayerController.h"
AFoodPlant::AFoodPlant()
{
    bReplicates=true;bAlwaysRelevant=true;SetReplicateMovement(true);
    PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.25f;
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EdibleCycad"));SetRootComponent(Visual);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);Visual->SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Fern(TEXT("/Game/World/SM_Fern.SM_Fern"));
    if(Fern.Succeeded())Visual->SetStaticMesh(Fern.Object);
    Visual->SetRelativeScale3D(FVector(3));
    Visual->SetCustomDepthStencilValue(2);
}
void AFoodPlant::BeginPlay()
{
    Super::BeginPlay();GConfig->GetFloat(TEXT("Dino.Food"),TEXT("PlantFoodUnits"),MaximumNutrition,GGameIni);
    GConfig->GetFloat(TEXT("Dino.Food"),TEXT("PlantRegrowSeconds"),RegrowSeconds,GGameIni);if(HasAuthority())Nutrition=MaximumNutrition;
}
float AFoodPlant::Consume(float Amount)
{
    if(!HasAuthority())return 0;
    float Taken=FMath::Clamp(Amount,0.f,Nutrition);Nutrition-=Taken;
    if(Nutrition<=0){RegrowTimer=RegrowSeconds;Visual->SetRenderCustomDepth(false);SetActorHiddenInGame(true);}return Taken;
}
void AFoodPlant::Tick(float Dt)
{
    Super::Tick(Dt);if(HasAuthority()&&Nutrition<=0&&RegrowSeconds>0){RegrowTimer-=Dt;if(RegrowTimer<=0){Nutrition=MaximumNutrition;SetActorHiddenInGame(false);}}
    const auto* PC=GetWorld()->GetFirstPlayerController();
    const auto* Player=PC?Cast<ADinosaurCharacter>(PC->GetPawn()):nullptr;
    Visual->SetRenderCustomDepth(IsAvailable()&&Player&&Player->Species==2&&!Player->bDead);
}
ADinosaurCarcass::ADinosaurCarcass()
{
    bReplicates=true;bAlwaysRelevant=true;SetReplicateMovement(true);
    PrimaryActorTick.bCanEverTick=false;
    auto* Root=CreateDefaultSubobject<USceneComponent>(TEXT("CarcassRoot"));SetRootComponent(Root);
    Body=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CarcassBody"));Body->SetupAttachment(Root);
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);Body->SetCanEverAffectNavigation(false);
}
void ADinosaurCarcass::Initialize(ADinosaurCharacter* Source)
{
    if(!HasAuthority())return;
    if(!Source){Destroy();return;}
    Species=Source->Species;SourceID=Source->CombatantID;Nutrition=MaximumNutrition=Source->Stats().FoodUnits;
    FVector Ground=Source->GetActorLocation();Ground.Z=ALostValleyWorld::HeightAt(Ground.X,Ground.Y);
    FVector Normal=FVector::UpVector;float Surface=0;
    const bool InWater=ALostValleyWorld::WaterAt(Ground.X,Ground.Y,Surface)&&Surface-Ground.Z>Source->Stats().HalfHeight*.5f;
    if(InWater)Ground.Z=Surface-Source->Stats().HalfHeight*.35f;
    else
    {
        // Use the real collision surface (including rocks), not just the analytic height field.
        FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(CarcassGround),false,Source);Query.AddIgnoredActor(this);
        if(GetWorld()->LineTraceSingleByObjectType(Hit,Source->GetActorLocation()+FVector(0,0,100),Ground-FVector(0,0,1500),FCollisionObjectQueryParams(ECC_WorldStatic),Query))
        {Ground=Hit.ImpactPoint;if(Hit.ImpactNormal.Z>.55f)Normal=Hit.ImpactNormal;}
    }
    const FRotator Rotation=FRotationMatrix::MakeFromZX(Normal,Source->GetActorForwardVector()).Rotator();
    SetActorLocationAndRotation(Ground+Normal*Source->Stats().HalfHeight,Rotation);
    // Never inherit a listen-server network-smoothing translation or rotation.
    BodyTransform=FTransform(FRotator::ZeroRotator,FVector(0,0,-Source->Stats().HalfHeight));
    Body->SetRelativeTransform(BodyTransform);Body->SetCullDistance(14000);Body->SetForcedLOD(3);
    Body->SetSkeletalMesh(Cast<USkeletalMesh>(Source->GetMesh()->GetSkinnedAsset()));
    for(int32 I=0;I<Source->GetMesh()->GetNumMaterials();++I)Body->SetMaterial(I,Source->GetMesh()->GetMaterial(I));
    const FString Name=Source->Stats().AssetName,A=Name+TEXT("_Death");
    auto* Clip=LoadObject<UAnimSequence>(nullptr,*(TEXT("/Game/Dinosaurs/")+Name+TEXT("/")+A+TEXT(".")+A));
    Body->SetAnimationMode(EAnimationMode::AnimationSingleNode);if(Clip)Body->PlayAnimation(Clip,false);
    Body->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    // Finish collapse even offscreen, then retain a frozen low-cost pose until eaten.
    FTimerHandle Freeze;GetWorldTimerManager().SetTimer(Freeze,FTimerDelegate::CreateWeakLambda(this,[this](){Body->SetComponentTickEnabled(false);}),Clip?Clip->GetPlayLength()+.2f:1.f,false);
}
float ADinosaurCarcass::Consume(float Amount)
{
    if(!HasAuthority())return 0;
    float Taken=FMath::Clamp(Amount,0.f,Nutrition);Nutrition-=Taken;if(Nutrition<=0)Destroy();return Taken;
}
UFoodInteractionComponent::UFoodInteractionComponent(){PrimaryComponentTick.bCanEverTick=true;SetIsReplicatedByDefault(true);}
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
        for(TActorIterator<ADinosaurCarcass> It(GetWorld());It;++It)
        {
            float Dist=FVector::DistSquared2D(It->GetActorLocation(),D->GetActorLocation());
            if(It->Nutrition>0&&Dist<BestDist){Best=*It;BestDist=Dist;}
        }
    }
    return Best;
}
bool UFoodInteractionComponent::StartEating()
{
    if(!GetOwner()->HasAuthority())return false;
    auto* D=Cast<ADinosaurCharacter>(GetOwner());if(!D||D->bDead||D->Combat->bBracing||D->Combat->IsBusy()||D->Combat->bCharging||D->GetCharacterMovement()->IsFalling())return false;
    if(GetWorld()->GetTimeSeconds()-D->Health->LastDamageTime<D->Stats().EatSafeDelay)return false;
    Source=FindFood(D->Stats().AttackRange+260);bEating=Source.IsValid();return bEating;
}
void UFoodInteractionComponent::StopEating(){if(!GetOwner()->HasAuthority())return;bEating=false;Source=nullptr;}
void UFoodInteractionComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);if(!GetOwner()->HasAuthority()||!bEating)return;
    auto* D=Cast<ADinosaurCharacter>(GetOwner());AActor* A=Source.Get();
    if(!D||D->bDead||!A||D->Combat->bBracing||D->Combat->IsBusy()||FVector::Dist2D(D->GetActorLocation(),A->GetActorLocation())>D->Stats().AttackRange+300){StopEating();return;}
    const float Requested=Dt*D->Stats().EatUnitsPerSecond;float Consumed=0;
    if(auto* Plant=Cast<AFoodPlant>(A))Consumed=Plant->Consume(Requested);
    if(auto* Corpse=Cast<ADinosaurCarcass>(A))Consumed=Corpse->Consume(Requested);
    if(Consumed<=0){StopEating();return;}
    const float FeedingTime=Consumed/FMath::Max(1.f,D->Stats().EatUnitsPerSecond);
    D->GetCharacterMovement()->StopMovementImmediately();
    D->Health->Heal(D->Health->Maximum*EatRate*FeedingTime);
    D->Stamina->Restore(D->Stats().EatStaminaRate*FeedingTime);
    D->Hunger->Restore(D->Stats().EatHungerRate*FeedingTime);FoodConsumed+=Consumed;
    if(D->Health->Fraction()>=1&&D->Stamina->Fraction()>=1&&D->Hunger->Fraction()>=1)StopEating();
}

void AFoodPlant::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(AFoodPlant,Nutrition);DOREPLIFETIME(AFoodPlant,MaximumNutrition);}
void AFoodPlant::OnRep_Nutrition(){SetActorHiddenInGame(Nutrition<=0);if(Nutrition<=0)Visual->SetRenderCustomDepth(false);}
void ADinosaurCarcass::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(ADinosaurCarcass,Nutrition);DOREPLIFETIME(ADinosaurCarcass,MaximumNutrition);DOREPLIFETIME(ADinosaurCarcass,Species);DOREPLIFETIME(ADinosaurCarcass,SourceID);DOREPLIFETIME(ADinosaurCarcass,BodyTransform);}
void ADinosaurCarcass::BeginPlay()
{
    Super::BeginPlay();
    // RepNotify can run before component BeginPlay enables ticking.
    if(!HasAuthority())OnRep_Carcass();
}
void ADinosaurCarcass::OnRep_Carcass()
{
    const FString Name=FSpeciesData::Get(Species).AssetName,A=Name+TEXT("_Death");
    Body->SetRelativeTransform(BodyTransform);Body->SetCullDistance(14000);Body->SetForcedLOD(3);
    Body->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,*(TEXT("/Game/Dinosaurs/")+Name+TEXT("/")+Name+TEXT(".")+Name)));
    auto* Clip=LoadObject<UAnimSequence>(nullptr,*(TEXT("/Game/Dinosaurs/")+Name+TEXT("/")+A+TEXT(".")+A));
    Body->SetAnimationMode(EAnimationMode::AnimationSingleNode);if(Clip){Body->PlayAnimation(Clip,false);Body->SetPosition(Clip->GetPlayLength(),false);Body->TickAnimation(0,false);Body->RefreshBoneTransforms();}
    Body->SetComponentTickEnabled(false);
}
void UFoodInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(UFoodInteractionComponent,bEating);}
