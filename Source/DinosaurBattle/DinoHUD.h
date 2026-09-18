#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DinoHUD.generated.h"
class UTexture2D;
class ADinosaurCharacter;
class ADinoPlayerController;
UCLASS()
class DINOSAURBATTLE_API ADinoHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
private:
    UPROPERTY() UTexture2D* WorldMap=nullptr;
    bool bMapLoaded=false;
    float Scale=1;
    void Text(const FString& Message,float X,float Y,float Size=1,FLinearColor Color=FLinearColor::White);
    void Panel(float X,float Y,float W,float H,float Alpha=.85f);
    void Bar(float X,float Y,float W,float H,float Fraction,FLinearColor Color);
    void DrawMenu(ADinosaurCharacter* D,ADinoPlayerController* PC);
    void DrawWorldMap(ADinosaurCharacter* D,bool Full);
};
