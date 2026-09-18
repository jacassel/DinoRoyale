#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DinoPlayerController.generated.h"
class ADinosaurCharacter;

UCLASS()
class DINOSAURBATTLE_API ADinoPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void PlayerTick(float Dt) override;
    virtual void SetupInputComponent() override;
    void SetMenuOpen(bool Open);
    void ToggleMenu();
    void ToggleMap();
    void ToggleHelp();
    void SelectRex();
    void SelectRaptor();
    void SelectTrike();
    void MenuClick();
    void ResumeGame();
    void QuitGame();
    void SensitivityUp();
    void SensitivityDown();
    void RespawnPlayer();
    void ToggleSettings();
    void ToggleBlood();
    bool bSettingsOpen=false,bBloodEnabled=false;
    UFUNCTION(Exec) void DinoSpecies(int32 Index);
    UFUNCTION(Exec) void DinoDamage(float Amount);
    UFUNCTION(Exec) void DinoHeal();
    UFUNCTION(Exec) void DinoTeleport(float X,float Y);
    UFUNCTION(Exec) void DinoSnapshot();
    bool bSelectionOpen=false,bMapOpen=false,bShowHelp=true;
    UPROPERTY() ADinosaurCharacter* TestTarget=nullptr;
private:
    bool bDevBridge=false;
    int32 LastSequence=0;
    double LastBridgeTime=0;
    double FrameSum=0;
    int32 FrameCount=0;
    FString BridgeRoot;
    void ReadBridge();
    void WriteTelemetry();
};
