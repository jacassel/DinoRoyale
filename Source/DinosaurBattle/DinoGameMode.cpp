#include "DinoGameMode.h"
#include "DinosaurCharacter.h"
#include "DinosaurAIController.h"
#include "LostValleyWorld.h"
#include "FoodSystem.h"
#include "DinoHUD.h"
#include "DinoPlayerController.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/TextureCube.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
ADinoGameMode::ADinoGameMode(){DefaultPawnClass=ADinosaurCharacter::StaticClass();HUDClass=ADinoHUD::StaticClass();PlayerControllerClass=ADinoPlayerController::StaticClass();}
void ADinoGameMode::BeginPlay()
{
    Super::BeginPlay();
    ALostValleyWorld* Valley=nullptr;
    for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
    if(!Valley)Valley=GetWorld()->SpawnActor<ALostValleyWorld>();
    auto* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,8000),FRotator(-32,-38,0));
    auto* Light=Cast<UDirectionalLightComponent>(Sun->GetLightComponent());
    Light->SetMobility(EComponentMobility::Movable);Light->SetIntensity(4.f);Light->SetLightColor(FLinearColor(1,.88f,.69f));
    Light->SetAtmosphereSunLight(true);Light->DynamicShadowDistanceMovableLight=20000;Light->DynamicShadowCascades=3;
    GetWorld()->SpawnActor<ASkyAtmosphere>();
    auto* Sky=GetWorld()->SpawnActor<ASkyLight>();Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sky->GetLightComponent()->SourceType=SLS_SpecifiedCubemap;Sky->GetLightComponent()->SetCubemap(LoadObject<UTextureCube>(nullptr,TEXT("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap")));Sky->GetLightComponent()->SetIntensity(1.2f);Sky->GetLightComponent()->SetRealTimeCaptureEnabled(false);
    auto* Fog=GetWorld()->SpawnActor<AExponentialHeightFog>();
    Fog->GetComponent()->SetFogDensity(.007f);Fog->GetComponent()->SetFogHeightFalloff(.18f);
    Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.40f,.56f,.62f));Fog->GetComponent()->SetStartDistance(6000);
    auto SpawnDino=[&](int32 Species,FVector P,int32 ID,bool Major)
    {
        P=Valley->NearestWalkable(P);P.Z+=FSpeciesData::Get(Species).HalfHeight+20;
        FTransform T(FRotator(0,ID*37,0),P);
        auto* D=GetWorld()->SpawnActorDeferred<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        D->Species=Species;D->CombatantID=ID;D->bMajor=Major;
        UGameplayStatics::FinishSpawningActor(D,T);D->HomePosition=D->GetActorLocation();
        auto* AI=GetWorld()->SpawnActor<ADinosaurAIController>();AI->Possess(D);
        return D;
    };
    const FVector Spawns[]={FVector(12500,3000,0),FVector(28500,-6000,0),FVector(-23500,13500,0),
        FVector(4500,4000,0),FVector(5000,4400,0),FVector(4400,4800,0),
        FVector(-14000,-22000,0),FVector(8500,-6500,0),FVector(22000,18000,0)};
    for(int32 I=0;I<9;++I)SpawnDino(I/3,Spawns[I],I+1,true);
    FRandomStream Random(7512);
    for(int32 I=0;I<18;++I)
    {
        FVector P=ALostValleyWorld::Landmarks()[I%6]+FVector(Random.FRandRange(-6500,6500),Random.FRandRange(-6500,6500),0);
        if(P.Size2D()<2700)P.X+=4000;
        SpawnDino(3,P,100+I,false);
    }
    for(const FVector& P:Valley->FeedingSpots)for(int32 I=0;I<4;++I)
    {
        FVector Spot=Valley->NearestWalkable(P+FVector(I%2*750,I/2*750,0));
        GetWorld()->SpawnActor<AFoodPlant>(Spot,FRotator(0,Random.FRandRange(0,360),0));
    }
    auto* PC=GetWorld()->GetFirstPlayerController();if(PC)
    {
        PC->SetControlRotation(FRotator(-13,0,0));PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=25;
        if(auto* D=Cast<ADinosaurCharacter>(PC->GetPawn())){D->SetActorLocation(Valley->GroundPoint(0,0,D->Stats().HalfHeight+25));D->HomePosition=D->GetActorLocation();}
    }
}
