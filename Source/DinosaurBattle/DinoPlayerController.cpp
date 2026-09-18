#include "DinoPlayerController.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Json.h"
#include "HighResScreenshot.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "EngineUtils.h"

void ADinoPlayerController::BeginPlay()
{
    Super::BeginPlay();
    bDevBridge=FParse::Param(FCommandLine::Get(),TEXT("DinoDevBridge"));
    BridgeRoot=FPaths::ProjectSavedDir()/TEXT("Automation");
    if(bDevBridge)IFileManager::Get().MakeDirectory(*BridgeRoot,true);
    SetInputMode(FInputModeGameOnly()); bShowMouseCursor=false;
}
void ADinoPlayerController::PlayerTick(float Dt)
{
    Super::PlayerTick(Dt);
    if(!bDevBridge)return;
    FrameSum+=Dt;FrameCount++;BridgeTimer+=Dt;
    if(BridgeTimer>=.05f){BridgeTimer=0;ReadBridge();WriteTelemetry();}
}
void ADinoPlayerController::DinoSpecies(int32 I){if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->ApplySpecies(I);D->ResetLife();}}
void ADinoPlayerController::DinoDamage(float A){if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))D->ReceiveHit(A,nullptr);}
void ADinoPlayerController::DinoHeal(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))D->Health->Heal(D->Health->Maximum);}
void ADinoPlayerController::DinoTeleport(float X,float Y)
{
    if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))
    {
        FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(D);
        float Z=500;
        if(GetWorld()->LineTraceSingleByChannel(Hit,FVector(X,Y,30000),FVector(X,Y,-30000),ECC_WorldStatic,Q))Z=Hit.ImpactPoint.Z+D->Stats().HalfHeight+10;
        D->SetActorLocation(FVector(X,Y,Z),false,nullptr,ETeleportType::TeleportPhysics);
        D->GetCharacterMovement()->StopMovementImmediately();
    }
}
void ADinoPlayerController::DinoSnapshot(){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/Windows/DinosaurBattle.png"),true,true);}
void ADinoPlayerController::ReadBridge()
{
    FString Text;if(!FFileHelper::LoadFileToString(Text,*(BridgeRoot/TEXT("command.json"))))return;
    TSharedPtr<FJsonObject> O;auto Reader=TJsonReaderFactory<>::Create(Text);
    if(!FJsonSerializer::Deserialize(Reader,O)||!O.IsValid())return;
    int32 Seq=O->GetIntegerField(TEXT("seq"));if(Seq<=LastSequence)return;LastSequence=Seq;
    FString Cmd=O->GetStringField(TEXT("cmd"));auto* D=Cast<ADinosaurCharacter>(GetPawn());if(!D)return;
    if(Cmd==TEXT("key"))
    {
        FKey Key(*O->GetStringField(TEXT("key"))); FString Event=O->GetStringField(TEXT("event"));
        float Value=1;double Number;if(O->TryGetNumberField(TEXT("value"),Number))Value=Number;
        FInputKeyEventArgs Args(nullptr,FInputDeviceId::CreateFromInternalId(0),Key,Event==TEXT("down")?IE_Pressed:Event==TEXT("up")?IE_Released:IE_Axis,Value,false,FPlatformTime::Cycles64());
        Args.DeltaTime=.05f;Args.NumSamples=1;InputKey(Args);
    }
    else if(Cmd==TEXT("species")){DinoSpecies(O->GetIntegerField(TEXT("value")));DinoTeleport(0,0);}
    else if(Cmd==TEXT("damage"))DinoDamage(O->GetNumberField(TEXT("value")));
    else if(Cmd==TEXT("heal"))DinoHeal();
    else if(Cmd==TEXT("respawn"))D->ResetLife();
    else if(Cmd==TEXT("teleport"))DinoTeleport(O->GetNumberField(TEXT("x")),O->GetNumberField(TEXT("y")));
    else if(Cmd==TEXT("face")){float Yaw=O->GetNumberField(TEXT("yaw"));SetControlRotation(FRotator(-13,Yaw,0));D->SetActorRotation(FRotator(0,Yaw,0));D->GetCharacterMovement()->StopMovementImmediately();}
    else if(Cmd==TEXT("target"))
    {
        if(!IsValid(TestTarget))
        {
            FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            TestTarget=GetWorld()->SpawnActor<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),D->GetActorLocation()+D->GetActorForwardVector()*D->Stats().AttackRange,FRotator::ZeroRotator,P);
        }
        if(TestTarget)
        {
            TestTarget->ApplySpecies(D->Species==0?2:0);TestTarget->ResetLife();TestTarget->bMajor=false;
            TestTarget->Health->Reset(10000,1000,0);
            TestTarget->SetActorLocation(D->GetActorLocation()+D->GetActorForwardVector()*(D->Stats().AttackRange*.75f));
            TestTarget->SetActorRotation((D->GetActorLocation()-TestTarget->GetActorLocation()).Rotation());
        }
    }
    else if(Cmd==TEXT("screenshot"))DinoSnapshot();
    else if(Cmd==TEXT("quit"))ConsoleCommand(TEXT("quit"));
}
void ADinoPlayerController::WriteTelemetry()
{
    auto* D=Cast<ADinosaurCharacter>(GetPawn());if(!D)return;
    auto O=MakeShared<FJsonObject>();O->SetNumberField(TEXT("seq"),LastSequence);O->SetNumberField(TEXT("time"),GetWorld()->GetTimeSeconds());
    O->SetNumberField(TEXT("species"),D->Species);O->SetNumberField(TEXT("health"),D->Health->Current);O->SetNumberField(TEXT("maxHealth"),D->Health->Maximum);
    FVector L=D->GetActorLocation();O->SetNumberField(TEXT("x"),L.X);O->SetNumberField(TEXT("y"),L.Y);O->SetNumberField(TEXT("z"),L.Z);
    O->SetNumberField(TEXT("speed"),D->GetVelocity().Size2D());O->SetNumberField(TEXT("maxSpeed"),D->GetCharacterMovement()->MaxWalkSpeed);
    O->SetNumberField(TEXT("yaw"),D->GetActorRotation().Yaw);O->SetNumberField(TEXT("cameraYaw"),GetControlRotation().Yaw);O->SetNumberField(TEXT("cameraPitch"),GetControlRotation().Pitch);
    O->SetBoolField(TEXT("brace"),D->Combat->bBracing);O->SetBoolField(TEXT("charging"),D->Combat->bCharging);O->SetBoolField(TEXT("dead"),D->bDead);
    O->SetBoolField(TEXT("falling"),D->GetCharacterMovement()->IsFalling());O->SetNumberField(TEXT("charge"),D->Combat->ChargeFraction());
    O->SetNumberField(TEXT("recovery"),D->Combat->RecoveryLeft);O->SetNumberField(TEXT("attackDuration"),D->Combat->AttackDuration);O->SetNumberField(TEXT("hits"),D->Combat->TotalHits);
    O->SetNumberField(TEXT("damageDealt"),D->Combat->LastDealtDamage);
    O->SetNumberField(TEXT("targetHealth"),IsValid(TestTarget)?TestTarget->Health->Current:-1);
    O->SetNumberField(TEXT("meanFPS"),FrameSum>0?FrameCount/FrameSum:0);
    int32 Major=0;for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor)Major++;
    O->SetNumberField(TEXT("majorCount"),Major);
    FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(O,W);
    FFileHelper::SaveStringToFile(Out,*(BridgeRoot/TEXT("telemetry.json")));
}
