#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DinoPlayerController.generated.h"
class ADinosaurCharacter;

USTRUCT()
struct FDinoMapMarker
{
    GENERATED_BODY()
    UPROPERTY() int32 ID=-1;
    UPROPERTY() FVector Position=FVector::ZeroVector;
};

UCLASS()
class DINOSAURBATTLE_API ADinoPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UPROPERTY(Replicated) TArray<FDinoMapMarker> VisibleMapMarkers;
    void UpdateMapVisibility();
    virtual void PlayerTick(float Dt) override;
    virtual void SetupInputComponent() override;
    void SetMenuOpen(bool Open);
    void ToggleMenu();
    void ToggleMultiplayer();
    // Page 0: offline; 1: multiplayer; 2: host; 3: browser; 4: lobby/match.
    int32 OnlinePage=0,HostCapacity=10,SelectedSession=-1;
    bool bHostTeams=false,bHostBots=false,bHostPublic=true;
    FString LobbyStatus;
    void OnlineClick(float X,float Y);
    UFUNCTION(Server,Reliable) void ServerLobbyAction(uint8 Action,int32 Value);
    UFUNCTION(Client,Reliable) void ClientLobbyMessage(const FString& Message);
    UFUNCTION(Client,Reliable) void ClientMatchStarted();
    UFUNCTION(Server,Reliable) void ServerDisplayName(const FString& Name);
    void ChooseSpecies(int32 Index);
    void ToggleMap();
    void PlaceMapPin();
    TArray<FVector> MapPins;
    void ToggleHelp();
    void SelectRex();
    void SelectRaptor();
    void SelectTrike();
    void MenuClick();
    void ResumeGame();
    void QuitGame();
    void SensitivityUp();
    void SensitivityDown();
    void ToggleSettings();
    void ToggleBlood();
    void ToggleNameTags();
    bool bShowNameTags=true;
    void ToggleMatchMode();
    bool bSettingsOpen=false,bBloodEnabled=false;
    UFUNCTION(Exec) void DinoSpecies(int32 Index);
    UFUNCTION(Exec) void DinoDamage(float Amount);
    UFUNCTION(Exec) void DinoHeal();
    UFUNCTION(Exec) void DinoTeleport(float X,float Y);
    UFUNCTION(Exec) void DinoSnapshot();
    bool bSelectionOpen=false,bMapOpen=false,bShowHelp=true;
    UPROPERTY() ADinosaurCharacter* TestTarget=nullptr;
    UPROPERTY() AActor* TestSightBlocker=nullptr;
private:
    bool bDevBridge=false,bWaitingForRoundStart=false;
    int32 LastSequence=0;
    double LastBridgeTime=0;
    double FrameSum=0;
    int32 FrameCount=0;
    FString BridgeRoot;
    void ReadBridge();
    void WriteTelemetry();
};
