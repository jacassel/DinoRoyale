#include "DinoGameMode.h"
#include "DinosaurCharacter.h"
#include "DinoHUD.h"
#include "DinoPlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
ADinoGameMode::ADinoGameMode(){DefaultPawnClass=ADinosaurCharacter::StaticClass();HUDClass=ADinoHUD::StaticClass();PlayerControllerClass=ADinoPlayerController::StaticClass();}
void ADinoGameMode::BeginPlay()
{
    Super::BeginPlay();
    auto* Floor=GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0,0,-100),FRotator::ZeroRotator);
    Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Floor->SetActorScale3D(FVector(500,500,2));
    auto* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,8000),FRotator(-40,-30,0));
    Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);Sun->GetLightComponent()->SetIntensity(3.f);
    auto* Sky=GetWorld()->SpawnActor<ASkyLight>();Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sky->GetLightComponent()->SetIntensity(1.f);
    for(int32 I=0;I<3;++I)
    {
        auto* D=GetWorld()->SpawnActor<ADinosaurCharacter>(FVector(1800,I*1200-1200,300),FRotator(0,180,0));
        D->ApplySpecies(I);D->CombatantID=I+1;
    }
    auto* PC=GetWorld()->GetFirstPlayerController();if(PC)
    {
        PC->SetControlRotation(FRotator(-13,0,0));
        PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=25;
        if(PC->GetPawn())PC->GetPawn()->SetActorLocation(FVector(0,0,300));
    }
}
