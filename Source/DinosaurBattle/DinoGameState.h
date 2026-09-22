#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DinoGameState.generated.h"
class ADinosaurCharacter;

USTRUCT()
struct FDinoScore
{
    GENERATED_BODY()
    UPROPERTY() int32 Kills=0;
    UPROPERTY() int32 Deaths=0;
    UPROPERTY() int32 Assists=0;
};
USTRUCT()
struct FDinoScoreRow
{
    GENERATED_BODY()
    UPROPERTY() int32 ID=0;
    UPROPERTY() FDinoScore Score;
};

/** Read-only replicated match view. Only GameMode changes the rules and scores. */
UCLASS()
class DINOSAURBATTLE_API ADinoGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    ADinoGameState();
    void SynchronizeRules();
    virtual void Tick(float Dt) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UPROPERTY(Replicated) bool bTeamMatch=false;
    UPROPERTY(Replicated) bool bRoundOver=false;
    UPROPERTY(Replicated) bool bLobby=false;
    UPROPERTY(Replicated) bool bFillBots=false;
    UPROPERTY(Replicated) int32 MaxParticipants=10;
    UPROPERTY(Replicated) int32 SoloKillGoal=5;
    UPROPERTY(Replicated) int32 TeamKillGoal=10;
    UPROPERTY(Replicated) TArray<int32> TeamKills={0,0};
    UPROPERTY(Replicated) int32 WinnerID=-1;
    UPROPERTY(Replicated) int32 WinnerTeam=-1;
    UPROPERTY(Replicated) int32 RoundNumber=0;
    UPROPERTY(ReplicatedUsing=OnRep_Scores) TArray<FDinoScoreRow> ScoreRows;
    TMap<int32,FDinoScore> Scores;
    UFUNCTION() void OnRep_Scores();
    FDinoScore GetScore(int32 ID) const;
    ADinosaurCharacter* FindCombatant(int32 ID) const;
    bool IsScoringTarget(const ADinosaurCharacter* D) const;
    FString MatchName() const;
    FString WinnerName() const;
};
