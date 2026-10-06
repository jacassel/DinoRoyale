#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DinoGameState.h"
#include "DinoGameMode.generated.h"
class ADinosaurCharacter;

/** Local match rules; health and abilities remain independent of scorekeeping. */
UCLASS()
class DINOSAURBATTLE_API ADinoGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ADinoGameMode();
    virtual void InitGame(const FString& Map,const FString& Options,FString& Error) override;
    virtual void PostLogin(APlayerController* PC) override;
    virtual void RestartPlayer(AController* C) override;
    virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* C,const FTransform& Transform) override;
    virtual void Logout(AController* C) override;
    virtual void PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& ID,FString& Error) override;
    virtual APlayerController* Login(UPlayer* NewPlayer,ENetRole InRemoteRole,const FString& Portal,const FString& Options,const FUniqueNetIdRepl& ID,FString& Error) override;
    virtual void BeginPlay() override;
    bool bTeamMatch=false,bRoundOver=false,bIgnoreWinCondition=false,bSharePackKills=true;
    bool bOnlineMatch=false,bLobby=false,bFillBots=false;
    int32 MaxParticipants=10;
    bool bPerformanceMap=false,bCustomBotSlots=false;
    TArray<int32> BotSlotTeams={0,0,0,0,0,1,1,1,1,1}; // -1 empty, 0 Team A, 1 Team B
    void SetMapVariant(bool Performance);
    void LobbyAction(class ADinoPlayerController* PC,uint8 Action,int32 Value);
    void StartNetworkRound();
    void ReturnToLobby();
    int32 ChooseTeam(int32 ExcludeID=-1) const;
    void UpdateLobby();
    void ReconcileBots();
    void RemoveParticipant(ADinosaurCharacter* Dino);
    FVector ParticipantHome(int32 ID,int32 Team) const;
    void SynchronizePacks();
    void RebuildPacks();
    bool bSynchronizingPacks=false;
    int32 SoloKillGoal=5,TeamKillGoal=10,TeamKills[2]={0,0},WinnerID=-1,WinnerTeam=-1,RoundNumber=0;
    int32 TeamAssists[2]={0,0};
    int32 TeamPoints(int32 Team) const {return Team>=0&&Team<2?TeamKills[Team]+TeamAssists[Team]/3:0;}
    float AssistWindow=12,RoundStartTime=0;
    TMap<int32,FDinoScore> Scores;
    void StartRound();
    void SetTeamMode(bool Enabled);
    bool AreEnemies(const ADinosaurCharacter* A,const ADinosaurCharacter* B) const;
    ADinosaurCharacter* GetPackLeader(const ADinosaurCharacter* Member) const;
    ADinosaurCharacter* ScoringOwner(ADinosaurCharacter* D) const;
    ADinosaurCharacter* FindCombatant(int32 ID) const;
    bool IsScoringTarget(const ADinosaurCharacter* D) const;
    void RegisterDamage(ADinosaurCharacter* Victim,ADinosaurCharacter* Attacker,float Amount);
    void RegisterDeath(ADinosaurCharacter* Victim);
    FDinoScore GetScore(int32 ID) const;
    FString MatchName() const;
    FString WinnerName() const;
};
