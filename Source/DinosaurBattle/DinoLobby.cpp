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
        const int32 Mask=UGameplayStatics::GetIntOption(Options,TEXT("AllowedSpecies"),247)&247;
        MatchRules.AllowedSpeciesMask=Mask?Mask:247;
        MatchRules.bExplicitBotCounts=UGameplayStatics::GetIntOption(Options,TEXT("ExplicitBots"),0)!=0;
        MatchRules.TeamABots=FMath::Clamp(UGameplayStatics::GetIntOption(Options,TEXT("TeamABots"),4),0,9);
        MatchRules.TeamBBots=FMath::Clamp(UGameplayStatics::GetIntOption(Options,TEXT("TeamBBots"),5),0,9);
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
    EnforceAllowedSpecies();ReconcileBots();
    SynchronizePacks();
    if(auto* GS=GetGameState<ADinoGameState>()){GS->SynchronizeRules();GS->ForceNetUpdate();}
    if(auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>())Online->UpdateHostedSettings(bTeamMatch,MaxParticipants,bFillBots);
}
void ADinoGameMode::LobbyAction(ADinoPlayerController* PC,uint8 Action,int32 Value)
{
    if(!PC)return;
    auto* PS=PC->GetPlayerState<ADinoPlayerState>();if(!PS)return;
    if(Action>=3&&GetNetMode()!=NM_Standalone&&!PS->bHost){PC->ClientLobbyMessage(TEXT("Only the host can change match settings."));return;}
    if(Action==7){ReturnToLobby();return;}
    if(Action==8&&bRoundOver){StartNetworkRound();return;}
    if(!bLobby&&GetNetMode()!=NM_Standalone){PC->ClientLobbyMessage(TEXT("Choices are available in the lobby."));return;}
    switch(Action)
    {
    case 0:
        if(!MatchRules.Allows(Value)){PC->ClientLobbyMessage(TEXT("That species is banned for this match."));return;}
        PS->SelectedSpecies=Value;PS->bReady=false;
        if(auto* D=Cast<ADinosaurCharacter>(PC->GetPawn())){D->ApplySpecies(Value);D->ResetLife();}
        break;
    case 1:
        if(!bTeamMatch)return;
        if(Value==-1)Value=ChooseTeam(PS->CombatantID);
        if(Value<0||Value>1)return;
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
    case 5:bFillBots=Value!=0;MatchRules.bExplicitBotCounts=false;break;
    case 13:
        if(Value!=5&&Value!=10&&Value!=15)return;
        TeamKillGoal=Value;break;
    case 14:
        if(!FSpeciesData::IsPlayable(Value))return;
        if((MatchRules.AllowedSpeciesMask^(1<<Value))==0){PC->ClientLobbyMessage(TEXT("Keep at least one species allowed."));return;}
        MatchRules.AllowedSpeciesMask^=1<<Value;
        EnforceAllowedSpecies();break;
    case 15:
    case 16:
        if(Value<0||Value>9)return;
        if(!MatchRules.bExplicitBotCounts)
        {
            MatchRules.TeamABots=MatchRules.TeamBBots=0;
            if(bFillBots)for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bFillerBot)
            {if(It->TeamID==0)++MatchRules.TeamABots;else if(It->TeamID==1)++MatchRules.TeamBBots;}
        }
        MatchRules.bExplicitBotCounts=true;
        if(Action==15)MatchRules.TeamABots=Value;else MatchRules.TeamBBots=Value;
        break;
    case 9:SetMapVariant(Value!=0);break;
    case 10:
    case 11:
        if(Value<0||Value>=MaxParticipants)return;
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P))if(Human->CombatantID==Value)return;
        if(!bFillBots)for(int32& Team:BotSlotTeams)Team=-1;
        MatchRules.bExplicitBotCounts=false;
        bCustomBotSlots=true;bFillBots=true;
        if(Action==10)BotSlotTeams[Value]=BotSlotTeams[Value]<0?0:-1;
        else if(bTeamMatch)BotSlotTeams[Value]=BotSlotTeams[Value]==0?1:0;
        break;
    case 12:
        MatchRules.bExplicitBotCounts=true;MatchRules.TeamABots=4;MatchRules.TeamBBots=5;
        bCustomBotSlots=false;bFillBots=true;MaxParticipants=10;bTeamMatch=true;
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P)){Human->bReady=false;if(Human->TeamID<0)Human->TeamID=ChooseTeam(Human->CombatantID);}
        break;
    case 6:
        for(auto P:GetGameState<ADinoGameState>()->PlayerArray)
            if(auto* Other=Cast<ADinoPlayerState>(P))if(!Other->bHost&&!Other->bReady){PC->ClientLobbyMessage(TEXT("Wait for each guest to mark Ready."));return;}
        StartNetworkRound();return;
    default:return;
    }
    if(GetNetMode()==NM_Standalone&&(Action==1||Action>=3))bOfflineRulesDirty=true;
    if(Action!=14)PC->ClientLobbyMessage(TEXT(""));PS->ForceNetUpdate();UpdateLobby();
}
void ADinoGameMode::StartNetworkRound()
{
    const FString Warning=ValidateSetup();
    if(!Warning.IsEmpty()){for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)if(auto* PC=Cast<ADinoPlayerController>(It->Get()))PC->ClientLobbyMessage(Warning);return;}
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

FString ADinoGameMode::ValidateSetup() const
{
    if(MatchRules.Allowed().IsEmpty())return TEXT("Keep at least one species allowed.");
    int32 Humans=0,Counts[2]={0,0};TSet<int32> HumanIDs;
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)if(auto* PS=It->Get()->GetPlayerState<ADinoPlayerState>())
    {++Humans;HumanIDs.Add(PS->CombatantID);if(PS->TeamID>=0&&PS->TeamID<2)++Counts[PS->TeamID];}
    if(bTeamMatch&&MatchRules.bExplicitBotCounts)
    {
        if(Humans+MatchRules.TeamABots+MatchRules.TeamBBots>MaxParticipants)return TEXT("Over capacity: reduce bots or increase slots. Maximum 10 combatants.");
        Counts[0]+=MatchRules.TeamABots;Counts[1]+=MatchRules.TeamBBots;
    }
    else if(bFillBots)for(int32 ID=0;ID<MaxParticipants;++ID)if(!HumanIDs.Contains(ID))
    {const int32 Team=bCustomBotSlots?BotSlotTeams[ID]:Counts[0]<=Counts[1]?0:1;if(Team>=0&&Team<2)++Counts[Team];}
    if(bTeamMatch&&(Counts[0]==0||Counts[1]==0))return TEXT("Each team needs at least one human or bot.");
    return TEXT("");
}
void ADinoGameMode::EnforceAllowedSpecies()
{
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)if(auto* PS=It->Get()->GetPlayerState<ADinoPlayerState>())
    {
        if(!MatchRules.Allows(PS->SelectedSpecies))
        {PS->SelectedSpecies=MatchRules.Resolve(PS->SelectedSpecies);PS->bReady=false;PS->ForceNetUpdate();if(auto* PC=Cast<ADinoPlayerController>(It->Get()))PC->ClientLobbyMessage(TEXT("Your species was banned; an allowed dinosaur has been selected."));}
    }
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor&&!MatchRules.Allows(It->Species))
    {if(It->bPackFollower)RemoveParticipant(*It);else It->ApplySpecies(It->bFillerBot?MatchRules.BotSpecies(It->CombatantID-1):MatchRules.Resolve(It->Species));}
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
