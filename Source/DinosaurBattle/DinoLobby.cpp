#include "DinoGameMode.h"
#include "DinoPlayerController.h"
#include "DinoPlayerState.h"
#include "DinoOnlineSession.h"
#include "DinosaurCharacter.h"
#include "DinosaurAIController.h"
#include "FoodSystem.h"
#include "LostValleyWorld.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameSession.h"

void ADinoGameMode::InitGame(const FString& Map,const FString& Options,FString& Error)
{
    Super::InitGame(Map,Options,Error);
    bOnlineMatch=UGameplayStatics::HasOption(Options,TEXT("OnlineLobby"));
    bLobby=bOnlineMatch;
    bPerformanceMap=UGameplayStatics::GetIntOption(Options,TEXT("PerformanceMap"),0)!=0;
    MaxParticipants=FMath::Clamp(UGameplayStatics::GetIntOption(Options,TEXT("Capacity"),10),2,10);
    if(GameSession){GameSession->MaxPlayers=MaxParticipants;GameSession->MaxSpectators=0;}
    if(bOnlineMatch)
    {
        bTeamMatch=UGameplayStatics::GetIntOption(Options,TEXT("Teams"),0)!=0;
        bFillBots=UGameplayStatics::GetIntOption(Options,TEXT("Bots"),0)!=0;
    }
}
int32 ADinoGameMode::ChooseTeam(int32 ExcludeID) const
{
    int32 Counts[2]={0,0};
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
        if(auto* PS=It->Get()->GetPlayerState<ADinoPlayerState>())
            if(PS->CombatantID!=ExcludeID&&PS->TeamID>=0&&PS->TeamID<2)++Counts[PS->TeamID];
    return Counts[0]<=Counts[1]?0:1;
}
void ADinoGameMode::UpdateLobby()
{
    if(GameSession){GameSession->MaxPlayers=MaxParticipants;GameSession->MaxSpectators=0;}
    ReconcileBots();
    SynchronizePacks();
    if(auto* GS=GetGameState<ADinoGameState>()){GS->SynchronizeRules();GS->ForceNetUpdate();}
    if(auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>())Online->UpdateHostedSettings(bTeamMatch,MaxParticipants,bFillBots);
}
void ADinoGameMode::LobbyAction(ADinoPlayerController* PC,uint8 Action,int32 Value)
{
    if(!PC||GetNetMode()==NM_Standalone)return;
    auto* PS=PC->GetPlayerState<ADinoPlayerState>();if(!PS)return;
    if(Action>=3&&!PS->bHost){PC->ClientLobbyMessage(TEXT("Only the host can change match settings."));return;}
    if(Action==7){ReturnToLobby();return;}
    if(Action==8&&bRoundOver){StartNetworkRound();return;}
    if(!bLobby){PC->ClientLobbyMessage(TEXT("Choices are available in the lobby."));return;}
    switch(Action)
    {
    case 0:
        if(!FSpeciesData::IsPlayable(Value))return;
        PS->SelectedSpecies=Value;PS->bReady=false;
        if(auto* D=Cast<ADinosaurCharacter>(PC->GetPawn())){D->ApplySpecies(Value);D->ResetLife();}
        break;
    case 1:
        if(!bTeamMatch)return;
        if(Value==-1)Value=ChooseTeam(PS->CombatantID);
        if(Value<0||Value>1)return;
        {int32 Count=0;for(auto P:GetGameState<ADinoGameState>()->PlayerArray)
            if(auto* Other=Cast<ADinoPlayerState>(P))if(Other!=PS&&Other->TeamID==Value)++Count;
        if(Count>=5){PC->ClientLobbyMessage(TEXT("That team is full (maximum five)."));return;}}
        PS->TeamID=Value;PS->bReady=false;break;
    case 2:PS->bReady=Value!=0;break;
    case 3:
        bTeamMatch=Value!=0;
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Other=Cast<ADinoPlayerState>(P)){Other->TeamID=-1;Other->bReady=false;}
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Other=Cast<ADinoPlayerState>(P))Other->TeamID=bTeamMatch?ChooseTeam(Other->CombatantID):-1;
        break;
    case 4:
        if(Value<2||Value>10||Value<GetNumPlayers()){PC->ClientLobbyMessage(TEXT("Capacity must fit connected players (2 to 10)."));return;}
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P))if(Human->CombatantID>=Value){PC->ClientLobbyMessage(TEXT("That slot is occupied by a connected player."));return;}
        MaxParticipants=Value;break;
    case 5:bFillBots=Value!=0;break;
    case 9:SetMapVariant(Value!=0);break;
    case 10:
    case 11:
        if(Value<0||Value>=MaxParticipants)return;
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P))if(Human->CombatantID==Value)return;
        if(!bFillBots)for(int32& Team:BotSlotTeams)Team=-1;
        bCustomBotSlots=true;bFillBots=true;
        if(Action==10)BotSlotTeams[Value]=BotSlotTeams[Value]<0?0:-1;
        else if(bTeamMatch)BotSlotTeams[Value]=BotSlotTeams[Value]==0?1:0;
        break;
    case 12:
        bCustomBotSlots=false;bFillBots=true;MaxParticipants=10;bTeamMatch=true;
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P)){Human->bReady=false;if(Human->TeamID<0)Human->TeamID=ChooseTeam(Human->CombatantID);}
        break;
    case 6:
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)
            if(auto* Other=Cast<ADinoPlayerState>(P))if(!Other->bHost&&!Other->bReady){PC->ClientLobbyMessage(TEXT("Wait for each guest to mark Ready."));return;}
        StartNetworkRound();return;
    default:return;
    }
    PC->ClientLobbyMessage(TEXT(""));PS->ForceNetUpdate();UpdateLobby();
}
void ADinoGameMode::StartNetworkRound()
{
    bLobby=false;bRoundOver=false;WinnerID=WinnerTeam=-1;TeamKills[0]=TeamKills[1]=0;TeamAssists[0]=TeamAssists[1]=0;Scores.Empty();++RoundNumber;RoundStartTime=GetWorld()->GetTimeSeconds();
    ReconcileBots();
    for(TActorIterator<ADinosaurCarcass> It(GetWorld());It;++It)It->Destroy();
    for(TActorIterator<AFoodPlant> It(GetWorld());It;++It){It->Nutrition=It->MaximumNutrition;It->OnRep_Nutrition();}
    int32 TeamSlots[2]={0,0};
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        auto* PC=Cast<ADinoPlayerController>(It->Get());auto* PS=PC?PC->GetPlayerState<ADinoPlayerState>():nullptr;
        auto* D=PC?Cast<ADinosaurCharacter>(PC->GetPawn()):nullptr;if(!PS||!D)continue;
        if(bTeamMatch&&PS->TeamID<0)PS->TeamID=ChooseTeam(PS->CombatantID);
        D->TeamID=bTeamMatch?PS->TeamID:-1;D->ApplySpecies(PS->SelectedSpecies);
        const float Angle=PS->CombatantID*2*PI/MaxParticipants;
        D->HomePosition=bTeamMatch?FVector(D->TeamID==0?-6000:6000,(TeamSlots[D->TeamID]++-2)*1400,0):FVector(FMath::Cos(Angle)*6500,FMath::Sin(Angle)*6500,0);
        D->bDead=true;D->ResetLife();D->GetCharacterMovement()->StopMovementImmediately();D->bScoringParticipant=true;
        Scores.Add(D->CombatantID,FDinoScore());PS->bReady=false;PC->ClientMatchStarted();
    }
    for(TActorIterator<ADinosaurAIController> It(GetWorld());It;++It){It->ResetTactics();It->ClearTravelGoal();}
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bFillerBot)
    {It->HomePosition=ParticipantHome(It->CombatantID,It->TeamID);It->bDead=true;It->ResetLife();Scores.Add(It->CombatantID,FDinoScore());}
    RebuildPacks();
    UpdateLobby();
}
void ADinoGameMode::ReturnToLobby()
{
    bLobby=true;bRoundOver=false;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It){It->CancelActions();It->GetCharacterMovement()->StopMovementImmediately();}
    for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* PS=Cast<ADinoPlayerState>(P))PS->bReady=false;
    UpdateLobby();
}

void ADinoGameMode::SetMapVariant(bool Performance)
{
    bPerformanceMap=Performance;
    for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){It->SetPerformanceMap(Performance);break;}
    if(GetNetMode()==NM_Standalone){GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("PerformanceMap"),Performance,GGameIni);GConfig->Flush(false,GGameIni);}
    if(auto* GS=GetGameState<ADinoGameState>()){GS->SynchronizeRules();GS->ForceNetUpdate();}
}
