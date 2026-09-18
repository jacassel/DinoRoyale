#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DinoHUD.generated.h"
UCLASS()
class DINOSAURBATTLE_API ADinoHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
