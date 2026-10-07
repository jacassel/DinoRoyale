#include "DinoPlayerController.h"
#include "DinoHUD.h"
#include "DinoGameMode.h"
#include "DinoGameState.h"
#include "DinoPlayerState.h"
#include "Engine/Canvas.h"

void ADinoPlayerController::MatchSetupClick(float X,float Y)
{
    auto* GS=GetWorld()->GetGameState<ADinoGameState>();if(!GS)return;
    auto At=[&](float A,float B,float W,float H){return X>=A&&X<=A+W&&Y>=B&&Y<=B+H;};
    if(At(.08f,.89f,.40f,.06f)){ToggleMatchSetup();return;}
    const auto* PS=GetPlayerState<ADinoPlayerState>();
    if(GetNetMode()!=NM_Standalone&&(!PS||!PS->bHost||!GS->bLobby))return;
    auto Change=[&](uint8 Action,int32 Value){ServerLobbyAction(Action,Value);};
    if(At(.08f,.14f,.40f,.06f)){Change(3,!GS->bTeamMatch);return;}
    if(At(.52f,.14f,.17f,.06f)){Change(4,GS->MaxParticipants-1);return;}
    if(At(.71f,.14f,.17f,.06f)){Change(4,GS->MaxParticipants+1);return;}
    if(GS->bTeamMatch)for(int32 I=0;I<3;++I)if(At(.08f+I*.27f,.25f,.25f,.055f)){Change(13,5+I*5);return;}
    for(int32 I=0;I<FSpeciesData::PlayableCount;++I)if(At(.08f+(I%4)*.21f,.38f+(I/4)*.065f,.195f,.055f)){Change(14,FSpeciesData::PlayableID(I));return;}
    if(GS->bTeamMatch)
    {
        for(int32 Team=0;Team<2;++Team)
        {
            const float Row=.58f+Team*.09f;int32 Count=Team==0?GS->MatchRules.TeamABots:GS->MatchRules.TeamBBots;
            if(!GS->MatchRules.bExplicitBotCounts)
            {
                Count=0;if(GS->bFillBots)for(int32 ID=0;ID<GS->BotSlotTeams.Num();++ID)
                {
                    bool HumanSlot=false;for(auto P:GS->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P))if(Human->CombatantID==ID)HumanSlot=true;
                    if(!HumanSlot&&GS->BotSlotTeams[ID]==Team)++Count;
                }
            }
            if(At(.57f,Row,.09f,.055f)){Change(15+Team,Count-1);return;}
            if(At(.79f,Row,.09f,.055f)){Change(15+Team,Count+1);return;}
        }
    }
    else if(At(.08f,.58f,.40f,.06f)){Change(5,!GS->bFillBots);return;}
}

void ADinoHUD::DrawMatchSetup(ADinoPlayerController* PC)
{
    auto* GS=GetWorld()->GetGameState<ADinoGameState>();if(!GS)return;
    const float W=Canvas->SizeX,H=Canvas->SizeY;
    const FLinearColor SetupGold(.93f,.72f,.38f),SetupTeal(.32f,.78f,.72f),SetupMuted(.64f,.72f,.69f);
    const auto* PS=PC->GetPlayerState<ADinoPlayerState>();
    const bool Editable=GetNetMode()==NM_Standalone||(PS&&PS->bHost&&GS->bLobby);
    DrawRect(FLinearColor(.009f,.019f,.022f,.98f),0,0,W,H);
    auto Label=[&](const FString& S,float X,float Y,float Size=.85f,FLinearColor C=FLinearColor::White){Text(S,W*X,H*Y,Size,C);};
    auto Button=[&](const FString& S,float X,float Y,float Width,float Height,bool Active=true)
    {
        DrawRect(Editable&&Active?FLinearColor(.07f,.17f,.18f):FLinearColor(.055f,.075f,.08f),W*X,H*Y,W*Width,H*Height);
        Label(S,X+.009f,Y+.012f,.76f,Editable&&Active?SetupTeal:SetupMuted);
    };
    Label(TEXT("MATCH SETUP"),.08f,.05f,1.5f,SetupGold);
    Label(Editable?TEXT("Host rules apply to humans, bots and respawns"):TEXT("Host controls these settings in the lobby"),.37f,.067f,.76f,SetupMuted);
    Button(GS->bTeamMatch?TEXT("MODE: TEAM BATTLE / CHANGE"):TEXT("MODE: FREE-FOR-ALL / CHANGE"),.08f,.14f,.40f,.06f);
    Button(FString::Printf(TEXT("CAPACITY %d / -"),GS->MaxParticipants),.52f,.14f,.17f,.06f);Button(TEXT("CAPACITY +"),.71f,.14f,.17f,.06f);
    Label(GS->bTeamMatch?TEXT("TEAM SCORE LIMIT"):TEXT("FFA SCORE LIMIT: 5 POINTS"),.08f,.218f,.75f,SetupGold);
    const TCHAR* Goals[]={TEXT("5 - SHORT"),TEXT("10 - STANDARD"),TEXT("15 - EXTENDED")};
    for(int32 I=0;I<3;++I)Button(FString(GS->TeamKillGoal==5+5*I?TEXT("[X] "):TEXT("[ ] "))+Goals[I],.08f+I*.27f,.25f,.25f,.055f,GS->bTeamMatch);
    Label(TEXT("ALLOWED SPECIES / CLICK TO ALLOW OR BAN"),.08f,.337f,.78f,SetupGold);
    for(int32 I=0;I<FSpeciesData::PlayableCount;++I)
    {
        const int32 ID=FSpeciesData::PlayableID(I);
        FString Name=ID==0?TEXT("T-REX"):ID==6?TEXT("PACHYCEPHALOSAUR"):FSpeciesData::Get(ID).Name.ToUpper();
        Button(FString(GS->MatchRules.Allows(ID)?TEXT("[X] "):TEXT("[ ] "))+Name,.08f+(I%4)*.21f,.38f+(I/4)*.065f,.195f,.055f);
    }
    int32 Humans[2]={0,0},HumanTotal=0;TSet<int32> HumanIDs;
    for(auto P:GS->PlayerArray)if(auto* Human=Cast<ADinoPlayerState>(P)){++HumanTotal;HumanIDs.Add(Human->CombatantID);if(Human->TeamID>=0&&Human->TeamID<2)++Humans[Human->TeamID];}
    int32 Bots[2]={0,0};
    if(GS->bTeamMatch&&GS->MatchRules.bExplicitBotCounts){Bots[0]=GS->MatchRules.TeamABots;Bots[1]=GS->MatchRules.TeamBBots;}
    else if(GS->bFillBots)for(int32 ID=0;ID<GS->MaxParticipants;++ID)if(!HumanIDs.Contains(ID)&&GS->BotSlotTeams.IsValidIndex(ID)){const int32 Team=GS->BotSlotTeams[ID];if(Team>=0&&Team<2)++Bots[Team];}
    Label(TEXT("TEAM SETUP / UNEQUAL TEAMS ARE ALLOWED"),.08f,.53f,.78f,SetupGold);
    if(GS->bTeamMatch)for(int32 Team=0;Team<2;++Team)
    {
        const float Row=.58f+Team*.09f;
        Label(FString::Printf(TEXT("TEAM %s:  %d humans + %d bots = %d total"),Team==0?TEXT("A"):TEXT("B"),Humans[Team],Bots[Team],Humans[Team]+Bots[Team]),.08f,Row+.014f,.88f);
        Button(TEXT("-"),.57f,Row,.09f,.055f);Label(FString::FromInt(Bots[Team]),.705f,Row+.012f,.9f,SetupTeal);Button(TEXT("+"),.79f,Row,.09f,.055f);
    }
    else Button(GS->bFillBots?TEXT("FILL AVAILABLE SLOTS: BOTS ON"):TEXT("FILL AVAILABLE SLOTS: BOTS OFF"),.08f,.58f,.40f,.06f);
    Label(FString::Printf(TEXT("MATCH TOTAL: %d / %d combatants maximum (pack followers use no slots)"),HumanTotal+Bots[0]+Bots[1],GS->MaxParticipants),.08f,.77f,.86f,SetupGold);
    Label(GS->SetupWarning.IsEmpty()?PC->LobbyStatus:GS->SetupWarning,.08f,.825f,.73f,SetupGold);
    Button(TEXT("ESC / F5 / BACK TO SELECTION"),.08f,.89f,.40f,.06f);
}
