#include "DinoGameState.h"
#include "DinoGameMode.h"
#include "DinosaurCharacter.h"
#include "DinoPlayerController.h"
#include "LostValleyWorld.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"

ADinoGameState::ADinoGameState(){PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.1f;}
void ADinoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADinoGameState,bPerformanceMap);DOREPLIFETIME(ADinoGameState,bCustomBotSlots);DOREPLIFETIME(ADinoGameState,BotSlotTeams);
    DOREPLIFETIME(ADinoGameState,bLobby);DOREPLIFETIME(ADinoGameState,MaxParticipants);DOREPLIFETIME(ADinoGameState,bFillBots);DOREPLIFETIME(ADinoGameState,bTeamMatch);DOREPLIFETIME(ADinoGameState,bRoundOver);
    DOREPLIFETIME(ADinoGameState,SoloKillGoal);DOREPLIFETIME(ADinoGameState,TeamKillGoal);
    DOREPLIFETIME(ADinoGameState,TeamKills);DOREPLIFETIME(ADinoGameState,WinnerID);
    DOREPLIFETIME(ADinoGameState,WinnerTeam);DOREPLIFETIME(ADinoGameState,RoundNumber);
    DOREPLIFETIME(ADinoGameState,ScoreRows);
}
void ADinoGameState::SynchronizeRules()
{
    if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())
    {
        bPerformanceMap=GM->bPerformanceMap;bCustomBotSlots=GM->bCustomBotSlots;BotSlotTeams=GM->BotSlotTeams;
        bLobby=GM->bLobby;MaxParticipants=GM->MaxParticipants;bFillBots=GM->bFillBots;bTeamMatch=GM->bTeamMatch;bRoundOver=GM->bRoundOver;SoloKillGoal=GM->SoloKillGoal;TeamKillGoal=GM->TeamKillGoal;
        TeamKills={GM->TeamKills[0],GM->TeamKills[1]};WinnerID=GM->WinnerID;WinnerTeam=GM->WinnerTeam;RoundNumber=GM->RoundNumber;
        Scores=GM->Scores;ScoreRows.Reset();
        TArray<int32> IDs;Scores.GetKeys(IDs);IDs.Sort();
        for(int32 ID:IDs){FDinoScoreRow R;R.ID=ID;R.Score=Scores[ID];ScoreRows.Add(R);}
    }
}
void ADinoGameState::Tick(float Dt)
{
    Super::Tick(Dt);SynchronizeRules();
    OnRep_MapVariant();
    if(HasAuthority()&&GetNetMode()!=NM_Standalone)for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
        if(auto* PC=Cast<ADinoPlayerController>(It->Get()))PC->UpdateMapVisibility();
    if(bRoundOver||bLobby)if(auto* PC=Cast<ADinoPlayerController>(GetWorld()->GetFirstPlayerController()))
        if(PC->IsLocalController()&&!PC->bSelectionOpen)PC->SetMenuOpen(true);
}
void ADinoGameState::OnRep_MapVariant()
{for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){It->SetPerformanceMap(bPerformanceMap);break;}}
void ADinoGameState::OnRep_Scores(){Scores.Reset();for(const auto& R:ScoreRows)Scores.Add(R.ID,R.Score);}
FDinoScore ADinoGameState::GetScore(int32 ID) const{if(const auto* S=Scores.Find(ID))return *S;return {};}
ADinosaurCharacter* ADinoGameState::FindCombatant(int32 ID) const
{for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor&&It->CombatantID==ID)return *It;return nullptr;}
bool ADinoGameState::IsScoringTarget(const ADinosaurCharacter* D) const
{return D&&D->bScoringParticipant;}
FString ADinoGameState::MatchName() const
{return bTeamMatch?FString::Printf(TEXT("TEAM BATTLE / FIRST TO %d"),TeamKillGoal):FString::Printf(TEXT("FREE-FOR-ALL / FIRST TO %d"),SoloKillGoal);}
FString ADinoGameState::WinnerName() const
{
    auto* PC=GetWorld()->GetFirstPlayerController();auto* Me=PC?Cast<ADinosaurCharacter>(PC->GetPawn()):nullptr;
    if(bTeamMatch)return Me&&Me->TeamID==WinnerTeam?TEXT("YOUR TEAM WINS"):TEXT("RIVAL TEAM WINS");
    if(Me&&Me->CombatantID==WinnerID)return TEXT("YOU WIN");
    auto* D=FindCombatant(WinnerID);return D?D->Stats().Name+TEXT(" WINS"):TEXT("ROUND COMPLETE");
}
