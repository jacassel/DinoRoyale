#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ADinosaurCharacter::ADinosaurCharacter()
{
    PrimaryActorTick.bCanEverTick=true;
    Health=CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
    Combat=CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
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
void ADinosaurCharacter::BeginPlay(){Super::BeginPlay();ApplySpecies(Species);}
void ADinosaurCharacter::ApplySpecies(int32 ID)
{
    const float OldHalf=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    Species=FMath::Clamp(ID,0,3); const auto& D=Stats();
    GetCapsuleComponent()->SetCapsuleSize(D.Radius,D.HalfHeight);
    if(HasActorBegunPlay()) AddActorWorldOffset(FVector(0,0,D.HalfHeight-OldHalf+3),false);
    GetCharacterMovement()->MaxWalkSpeed=D.Speed; GetCharacterMovement()->MaxAcceleration=D.Acceleration;
    GetCharacterMovement()->RotationRate=FRotator(0,D.TurnRate,0); GetCharacterMovement()->JumpZVelocity=D.JumpVelocity;
    CameraBoom->TargetArmLength=D.CameraDistance; CameraBoom->SocketOffset=FVector(0,0,D.CameraHeight);
    Placeholder->SetRelativeScale3D(FVector(D.AttackRange/140.f,D.Radius/55.f,D.HalfHeight/80.f));
    Health->Reset(D.MaxHealth,D.RegenDelay,D.RegenRate); Combat->Cancel(); bDead=false;
}
void ADinosaurCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    if(Health->IsDead()&&!bDead) Die();
    if(bDead){DeathTime+=Dt; if(DeathTime>6 && bMajor) ResetLife(); return;}
    auto* M=GetCharacterMovement();
    M->MaxWalkSpeed=Stats().Speed*Health->MovementFactor()*(Combat->bCharging?.7f:1.f);
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
    I->BindAction("Eat",IE_Pressed,this,&ADinosaurCharacter::Eat);
    I->BindAction("SelectRex",IE_Pressed,this,&ADinosaurCharacter::ChooseRex);
    I->BindAction("SelectRaptor",IE_Pressed,this,&ADinosaurCharacter::ChooseRaptor);
    I->BindAction("SelectTrike",IE_Pressed,this,&ADinosaurCharacter::ChooseTrike);
}
void ADinosaurCharacter::MoveForward(float V){if(!bDead&&!Combat->bBracing&&Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V);}
void ADinosaurCharacter::MoveRight(float V){if(!bDead&&!Combat->bBracing&&Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);}
void ADinosaurCharacter::Turn(float V){AddControllerYawInput(V*MouseSensitivity);}
void ADinosaurCharacter::Look(float V){AddControllerPitchInput(V*MouseSensitivity);}
void ADinosaurCharacter::BeginJump(){if(!bDead&&!Combat->bBracing&&!Combat->IsBusy()) Jump();}
void ADinosaurCharacter::BraceOn(){Combat->SetBrace(true);}
void ADinosaurCharacter::BraceOff(){Combat->SetBrace(false);}
void ADinosaurCharacter::Quick(){Combat->QuickAttack();}
void ADinosaurCharacter::ChargeOn(){Combat->StartCharge();}
void ADinosaurCharacter::ChargeOff(){Combat->ReleaseCharge();}
void ADinosaurCharacter::Eat(){}
bool ADinosaurCharacter::IsEnemy(const ADinosaurCharacter* O) const
{
    if(!O||O==this||O->bDead) return false;
    if(Species==1&&O->Species==1) return false;
    if(Species==2&&O->Species==2) return false;
    return true;
}
void ADinosaurCharacter::ReceiveHit(float Damage,ADinosaurCharacter* Attacker)
{
    if(bDead) return;
    if(Combat->bBracing&&Attacker)
    {
        FVector Dir=(Attacker->GetActorLocation()-GetActorLocation()).GetSafeNormal2D();
        if(FVector::DotProduct(GetActorForwardVector(),Dir)>.25f) Damage*=Stats().BraceMultiplier;
    }
    Health->Receive(Damage); if(Health->IsDead()) Die();
}
void ADinosaurCharacter::Die(){bDead=true;DeathTime=0;Combat->Cancel();GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();Placeholder->SetRelativeRotation(FRotator(0,0,75));}
void ADinosaurCharacter::ResetLife()
{
    bool WasDead=bDead;bDead=false;DeathTime=0;
    Health->Reset(Stats().MaxHealth,Stats().RegenDelay,Stats().RegenRate);Combat->Cancel();
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);Placeholder->SetRelativeRotation(FRotator::ZeroRotator);
    if(WasDead&&IsPlayerControlled())SetActorLocation(FVector(0,0,Stats().HalfHeight+20),false,nullptr,ETeleportType::TeleportPhysics);
}
void ADinosaurCharacter::ChooseRex(){ApplySpecies(0);ResetLife();}
void ADinosaurCharacter::ChooseRaptor(){ApplySpecies(1);ResetLife();}
void ADinosaurCharacter::ChooseTrike(){ApplySpecies(2);ResetLife();}
