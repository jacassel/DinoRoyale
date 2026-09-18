#include "DinoHUD.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
void ADinoHUD::DrawHUD()
{
    Super::DrawHUD();if(!Canvas)return;
    auto* D=Cast<ADinosaurCharacter>(GetOwningPawn());if(!D)return;
    DrawRect(FLinearColor(0.02f,.035f,.035f,.85f),24,24,440,135);
    DrawText(TEXT("DINOSAUR BATTLE  /  PRE-ALPHA 0.1"),FLinearColor(.95f,.79f,.42f),42,37,GEngine->GetMediumFont(),.9f);
    DrawText(D->Stats().Name,FLinearColor::White,42,63,GEngine->GetMediumFont(),1.2f);
    DrawRect(FLinearColor(.15f,.16f,.14f),42,103,396,15);
    DrawRect(D->Health->Fraction()<.25f?FLinearColor(.8f,.19f,.12f):FLinearColor(.45f,.76f,.42f),42,103,396*D->Health->Fraction(),15);
    FString Status=FString::Printf(TEXT("%.0f / %.0f  |  %s"),D->Health->Current,D->Health->Maximum,D->bDead?TEXT("RESPAWNING"):D->Combat->bBracing?TEXT("BRACED"):D->Health->Fraction()<.25f?TEXT("CRITICAL - CHARGE DISABLED"):D->Health->Fraction()<.5f?TEXT("INJURED"):TEXT("HEALTHY"));
    DrawText(Status,FLinearColor::White,42,129,GEngine->GetSmallFont());
    DrawText(TEXT("WASD move  |  Mouse look  |  Space jump  |  Hold Q brace  |  LMB bite  |  Hold / release RMB heavy"),FLinearColor::White,30,Canvas->SizeY-56,GEngine->GetSmallFont());
    DrawText(TEXT("1 T-Rex     2 Velociraptor     3 Triceratops     E eat"),FLinearColor(.9f,.8f,.55f),30,Canvas->SizeY-33,GEngine->GetSmallFont());
    if(D->Combat->bCharging)
    {
        DrawRect(FLinearColor(.04f,.04f,.04f,.8f),Canvas->SizeX/2-150,Canvas->SizeY-135,300,12);
        DrawRect(FLinearColor(1,.68f,.2f),Canvas->SizeX/2-150,Canvas->SizeY-135,300*D->Combat->ChargeFraction(),12);
        DrawText(TEXT("CHARGING - RELEASE RMB"),FLinearColor::White,Canvas->SizeX/2-100,Canvas->SizeY-158,GEngine->GetSmallFont());
    }
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* O=*It;if(O==D||O->bDead||FVector::DistSquared(D->GetActorLocation(),O->GetActorLocation())>FMath::Square(6000.f))continue;
        FVector2D Screen;
        if(PlayerOwner->ProjectWorldLocationToScreen(O->GetActorLocation()+FVector(0,0,O->Stats().HalfHeight+80),Screen))
        {
            DrawRect(FLinearColor(.05f,.05f,.05f,.8f),Screen.X-42,Screen.Y,84,6);
            DrawRect(FLinearColor(.8f,.3f,.15f),Screen.X-42,Screen.Y,84*O->Health->Fraction(),6);
        }
    }
}
