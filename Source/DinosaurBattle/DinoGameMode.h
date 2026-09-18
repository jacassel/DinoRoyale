#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DinoGameMode.generated.h"

UCLASS()
class DINOSAURBATTLE_API ADinoGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ADinoGameMode();
    virtual void BeginPlay() override;
};
