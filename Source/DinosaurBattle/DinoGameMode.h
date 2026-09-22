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
    virtual void PostLogin(APlayerController* PC) override;
    virtual void RestartPlayer(AController* C) override;
    virtual void Logout(AController* C) override;
    virtual void PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& ID,FString& Error) override;
    virtual void BeginPlay() override;
    bool bTeamMatch=false,bRoundOver=false,bIgnoreWinCondition=false,bSharePackKills=true;
    int32 SoloKillGoal=5,TeamKillGoal=10,TeamKills[2]={0,0},WinnerID=-1,WinnerTeam=-1,RoundNumber=0;
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
