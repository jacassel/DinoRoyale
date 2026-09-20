#include "DinosaurCharacter.h"
#include "DinoMovementComponent.h"
#include "DinoAudioComponent.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "HungerComponent.h"
#include "CombatComponent.h"
#include "DinoAnimationComponent.h"
#include "FoodSystem.h"
#include "DinosaurAIController.h"
#include "DinoGameMode.h"
#include "LostValleyWorld.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"

ADinosaurCharacter::ADinosaurCharacter(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer.SetDefaultSubobjectClass<UDinoMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick=true;
    Health=CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
    Hunger=CreateDefaultSubobject<UHungerComponent>(TEXT("Hunger"));
    Stamina=CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina"));
    Combat=CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
    Animation=CreateDefaultSubobject<UDinoAnimationComponent>(TEXT("Animation"));
    Food=CreateDefaultSubobject<UFoodInteractionComponent>(TEXT("Food"));
    Audio=CreateDefaultSubobject<UDinoAudioComponent>(TEXT("Audio"));
    CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent); CameraBoom->bUsePawnControlRotation=true;
    CameraBoom->bEnableCameraLag=true; CameraBoom->CameraLagSpeed=10;
    CameraBoom->CameraLagMaxDistance=90; CameraBoom->bUseCameraLagSubstepping=true;
    CameraBoom->ProbeSize=35; CameraBoom->bEnableCameraRotationLag=false;
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); Camera->SetupAttachment(CameraBoom);
    Camera->FieldOfView=80;
    Placeholder=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TemporaryBody"));
    Placeholder->SetupAttachment(RootComponent); Placeholder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Placeholder->SetStaticMesh(TempMesh.Object);
    bUseControllerRotationYaw=false;
    GetCharacterMovement()->bOrientRotationToMovement=true;
    GetCharacterMovement()->AirControl=.3f; GetCharacterMovement()->GravityScale=1.8f;
    GetCharacterMovement()->BrakingDecelerationWalking=2600;
    GetCharacterMovement()->GroundFriction=7;
    GetCharacterMovement()->MaxStepHeight=90;
    GetCharacterMovement()->SetWalkableFloorAngle(48);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Dinosaur"));
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);
}
void ADinosaurCharacter::BeginPlay(){Super::BeginPlay();HomePosition=GetActorLocation();GConfig->GetFloat(TEXT("Dino.Session"),TEXT("RespawnDelay"),RespawnDelay,GGameIni);GConfig->GetFloat(TEXT("Dino.Session"),TEXT("MouseSensitivity"),MouseSensitivity,GGameIni);GConfig->GetFloat(TEXT("Dino.Session"),TEXT("EatingHealRate"),Food->EatRate,GGameIni);GConfig->GetFloat(TEXT("Dino.Session"),TEXT("WaterSpeedMultiplier"),WaterSpeedMultiplier,GGameIni);ApplySpecies(Species);}
void ADinosaurCharacter::ApplySpecies(int32 ID)
{
    Audio->ResetAudio();
    const float OldHalf=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    Species=FMath::Clamp(ID,0,3); const auto& D=Stats();
    GetCapsuleComponent()->SetCapsuleSize(D.Radius,D.HalfHeight);
    if(HasActorBegunPlay()) AddActorWorldOffset(FVector(0,0,D.HalfHeight-OldHalf+3),false);
    GetCharacterMovement()->MaxWalkSpeed=D.Speed; GetCharacterMovement()->MaxAcceleration=D.Acceleration;
    GetCharacterMovement()->RotationRate=FRotator(0,D.TurnRate,0); GetCharacterMovement()->JumpZVelocity=D.JumpVelocity;
    CameraBoom->TargetArmLength=D.CameraDistance; CameraBoom->SocketOffset=FVector(0,0,D.CameraHeight);
    Placeholder->SetRelativeScale3D(FVector(D.AttackRange/140.f,D.Radius/55.f,D.HalfHeight/80.f));
    Health->Reset(D.MaxHealth,D.RegenDelay,D.RegenRate);Stamina->Reset();Hunger->Reset();RevealUntil=-100;SprintOff();GetMesh()->SetHiddenInGame(false); Combat->Cancel();Food->StopEating();bDead=false;
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Animation->LoadSpecies();
}
void ADinosaurCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    if(Health->IsDead()&&!bDead) Die();
    if(bDead){DeathTime+=Dt;if(RespawnDelay>0&&DeathTime>(bMajor?RespawnDelay:45))ResetLife();return;}
    auto* M=GetCharacterMovement();
    FVector Feet=GetActorLocation()-FVector(0,0,Stats().HalfHeight);
    const bool OverWater=ALostValleyWorld::WaterAt(Feet.X,Feet.Y,WaterSurface);
    const float Depth=OverWater?WaterSurface-ALostValleyWorld::HeightAt(Feet.X,Feet.Y):0;
    bInWater=OverWater&&Feet.Z<WaterSurface+15;
    const bool ShouldSwim=bInWater&&Depth>Stats().HalfHeight*(bSwimming?1.10f:1.30f)&&GetActorLocation().Z<WaterSurface+Stats().HalfHeight*.50f&&M->Velocity.Z<100;
    if(ShouldSwim&&!bSwimming){bSwimming=true;M->SetMovementMode(MOVE_Custom);}
    else if(bSwimming&&(!OverWater||Depth<Stats().HalfHeight*1.10f)){bSwimming=false;M->SetMovementMode(MOVE_Falling);}
    M->MaxSwimSpeed=Stats().Speed*SwimSpeedMultiplier*Health->MovementFactor()*(Combat->bCharging?.7f:1.f);
    bSprinting=bSprintRequested&&!Stamina->bExhausted&&Stamina->Current>0&&!bInWater&&!Combat->bBracing&&!Combat->bCharging&&!Combat->IsBusy()&&!Food->bEating&&!M->IsFalling()&&GetVelocity().Size2D()>50;
    if(bSprinting)RevealNoise();
    const float Commit=Combat->IsBusy()&&Combat->bChargedAttack?Stats().HeavyMoveFactor:1.f;
    M->MaxWalkSpeed=Stats().Speed*Health->MovementFactor()*(Combat->bCharging?.55f:Commit)*(bSprinting?Stats().SprintMultiplier:1.f)*(bInWater?WaterSpeedMultiplier:1.f);
    M->MaxAcceleration=Stats().Acceleration*(bSprinting?Stats().SprintAcceleration:1.f)*Commit;
    M->RotationRate=FRotator(0,Stats().TurnRate*TurnFactor(),0);
    if(Combat->bBracing){M->StopMovementImmediately(); ConsumeMovementInputVector();}
    if(Combat->bCharging&&Health->Fraction()<.25f) Combat->Cancel();
}
void ADinosaurCharacter::SetupPlayerInputComponent(UInputComponent* I)
{
    Super::SetupPlayerInputComponent(I);
    I->BindAxis("MoveForward",this,&ADinosaurCharacter::MoveForward); I->BindAxis("MoveRight",this,&ADinosaurCharacter::MoveRight);
    I->BindAxis("Turn",this,&ADinosaurCharacter::Turn); I->BindAxis("LookUp",this,&ADinosaurCharacter::Look);
    I->BindAction("Jump",IE_Pressed,this,&ADinosaurCharacter::BeginJump); I->BindAction("Jump",IE_Released,this,&ACharacter::StopJumping);
    I->BindAction("Brace",IE_Pressed,this,&ADinosaurCharacter::BraceOn); I->BindAction("Brace",IE_Released,this,&ADinosaurCharacter::BraceOff);
    I->BindAction("QuickAttack",IE_Pressed,this,&ADinosaurCharacter::Quick);
    I->BindAction("Charge",IE_Pressed,this,&ADinosaurCharacter::ChargeOn); I->BindAction("Charge",IE_Released,this,&ADinosaurCharacter::ChargeOff);
    I->BindAction("Sprint",IE_Pressed,this,&ADinosaurCharacter::SprintOn); I->BindAction("Sprint",IE_Released,this,&ADinosaurCharacter::SprintOff);
    I->BindAction("Eat",IE_Pressed,this,&ADinosaurCharacter::Eat);
    I->BindAction("Eat",IE_Released,this,&ADinosaurCharacter::StopEating);

}
void ADinosaurCharacter::MoveForward(float V){if(!FMath::IsNearlyZero(V))Food->StopEating();if(!bDead&&!Combat->bBracing&&Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V);}
void ADinosaurCharacter::MoveRight(float V){if(!FMath::IsNearlyZero(V))Food->StopEating();if(!bDead&&!Combat->bBracing&&Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);}
void ADinosaurCharacter::Turn(float V){AddControllerYawInput(V*MouseSensitivity);}
void ADinosaurCharacter::Look(float V){AddControllerPitchInput(V*MouseSensitivity);}
void ADinosaurCharacter::BeginJump(){if(!bDead&&!Combat->bBracing&&!Combat->bCharging&&!Combat->IsBusy()&&(bSwimming||GetCharacterMovement()->IsMovingOnGround())&&Stamina->Spend(Stats().JumpCost)){Food->StopEating();if(bSwimming){bSwimming=false;GetCharacterMovement()->SetMovementMode(MOVE_Falling);LaunchCharacter(FVector(0,0,Stats().JumpVelocity*.8f),false,true);}else Jump();}}
void ADinosaurCharacter::BraceOn(){Food->StopEating();Combat->SetBrace(true);}
void ADinosaurCharacter::BraceOff(){Combat->SetBrace(false);}
void ADinosaurCharacter::Quick(){Food->StopEating();Combat->QuickAttack();}
void ADinosaurCharacter::ChargeOn(){Food->StopEating();Combat->StartCharge();}
void ADinosaurCharacter::ChargeOff(){Combat->ReleaseCharge();}
void ADinosaurCharacter::Eat(){Food->StartEating();}
void ADinosaurCharacter::StopEating(){Food->StopEating();}
bool ADinosaurCharacter::IsEnemy(const ADinosaurCharacter* O) const
{
    if(!O||O==this||O->bDead) return false;
    if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())return GM->AreEnemies(this,O);
    if(Species==1&&O->Species==1)return false;
    return true;
}
void ADinosaurCharacter::ReceiveHit(float Damage,ADinosaurCharacter* Attacker)
{
    if(bDead) return;
    Food->StopEating();LastAttacker=Attacker;
    if(auto* AI=Cast<ADinosaurAIController>(GetController()))AI->Alert(Attacker);
    if(Combat->bBracing&&Attacker)
    {
        FVector Dir=(Attacker->GetActorLocation()-GetActorLocation()).GetSafeNormal2D();
        if(FVector::DotProduct(GetActorForwardVector(),Dir)>.25f){Stamina->Drain(Damage*Stats().BraceHitCost);Damage*=Stats().BraceMultiplier;if(Stamina->bExhausted)Combat->SetBrace(false);}
    }
    float Applied=Health->Receive(Damage);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->RegisterDamage(this,Attacker,Applied);if(Health->IsDead())Die();
}
void ADinosaurCharacter::Die(){if(bDead)return;if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->RegisterDeath(this);bDead=true;DeathTime=0;Audio->PlayEvent(4);if(auto* Corpse=GetWorld()->SpawnActor<ADinosaurCarcass>())Corpse->Initialize(this);GetMesh()->SetHiddenInGame(true);Placeholder->SetHiddenInGame(true);Combat->Cancel();Food->StopEating();GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);Placeholder->SetRelativeRotation(FRotator(0,0,75));}
void ADinosaurCharacter::ResetLife()
{
    Audio->ResetAudio();
    bool WasDead=bDead;bDead=false;bSwimming=false;bInWater=false;DeathTime=0;LastAttacker=nullptr;DamageContributors.Empty();Food->StopEating();
    Health->Reset(Stats().MaxHealth,Stats().RegenDelay,Stats().RegenRate);Stamina->Reset();Hunger->Reset();RevealUntil=-100;SprintOff();GetMesh()->SetHiddenInGame(false);Combat->Cancel();
    GConfig->GetFloat(TEXT("Dino.Session"),TEXT("SwimSpeedMultiplier"),SwimSpeedMultiplier,GGameIni);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);Placeholder->SetRelativeRotation(FRotator::ZeroRotator);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Placeholder->SetHiddenInGame(false);
    if(WasDead)
    {
        const FVector Home=HomePosition;
        ALostValleyWorld* Valley=nullptr;for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
        FVector P=Home;
        for(int32 Attempt=0;Attempt<33;++Attempt)
        {
            float A=Attempt*2.4f,R=Attempt==0?0:600.f+Attempt*80.f;
            P=Home+FVector(FMath::Cos(A)*R,FMath::Sin(A)*R,0);
            if(Valley&&!Valley->IsWalkable(P,Stats().Radius+70))continue;
            bool Clear=true;
            for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
                if(*It!=this&&!It->bDead&&FVector::Dist2D(P,It->GetActorLocation())<Stats().Radius+It->Stats().Radius+180){Clear=false;break;}
            if(Clear)break;
        }
        P.Z=ALostValleyWorld::HeightAt(P.X,P.Y)+Stats().HalfHeight+30;
        SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);
    }
}
void ADinosaurCharacter::ChooseRex(){ApplySpecies(0);ResetLife();}
void ADinosaurCharacter::ChooseRaptor(){ApplySpecies(1);ResetLife();}
void ADinosaurCharacter::ChooseTrike(){ApplySpecies(2);ResetLife();}

float ADinosaurCharacter::TurnFactor() const
{
    if(Combat->IsBusy()&&Combat->bChargedAttack)return Stats().HeavyTurnFactor;
    if(Combat->bCharging)return .5f;
    return bSprinting?Stats().SprintTurnFactor:1.f;
}

void ADinosaurCharacter::RevealNoise(){RevealUntil=GetWorld()->GetTimeSeconds()+Stats().NoiseRevealDuration;LastRevealedPosition=GetActorLocation();}
bool ADinosaurCharacter::CanSeeDinosaur(const ADinosaurCharacter* Other) const
{
    if(!Other||Other->bDead||bDead)return false;
    FVector Eye=GetActorLocation()+FVector(0,0,Stats().HalfHeight*.4f);FRotator View=GetActorRotation();float HalfAngle=60;
    if(auto* PC=Cast<APlayerController>(Controller)){PC->GetPlayerViewPoint(Eye,View);HalfAngle=Camera->FieldOfView*.5f;}
    const FVector Point=Other->GetActorLocation()+FVector(0,0,Other->Stats().HalfHeight*.35f);
    const FVector Delta=Point-Eye;if(Delta.SizeSquared()>FMath::Square(Stats().SightRange))return false;
    if(FVector::DotProduct(View.Vector(),Delta.GetSafeNormal())<FMath::Cos(FMath::DegreesToRadians(HalfAngle)))return false;
    FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(DinosaurSight),false,this);Q.AddIgnoredActor(Other);
    return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Point,ECC_Visibility,Q);
}
bool ADinosaurCharacter::MapPositionFor(const ADinosaurCharacter* Other,FVector& Position) const
{
    if(!Other||Other->bDead)return false;
    if(Other==this||CanSeeDinosaur(Other)){Position=Other->GetActorLocation();return true;}
    if(GetWorld()->GetTimeSeconds()<Other->RevealUntil){Position=Other->LastRevealedPosition;return true;}
    return false;
}
