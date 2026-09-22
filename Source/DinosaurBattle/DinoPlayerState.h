#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DinoPlayerState.generated.h"

UCLASS()
class DINOSAURBATTLE_API ADinoPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated) int32 CombatantID=-1;
    UPROPERTY(Replicated) int32 SelectedSpecies=0;
    UPROPERTY(Replicated) int32 TeamID=-1;
    UPROPERTY(Replicated) bool bReady=false;
    UPROPERTY(Replicated) bool bHost=false;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
