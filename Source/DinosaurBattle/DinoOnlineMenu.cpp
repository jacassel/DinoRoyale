#include "DinoPlayerController.h"
#include "DinoOnlineSession.h"
#include "DinoGameMode.h"
#include "DinoGameState.h"
#include "DinoPlayerState.h"
#include "DinosaurCharacter.h"
#include "DinoHUD.h"
#include "Engine/Canvas.h"

void ADinoPlayerController::ToggleMultiplayer()
{OnlinePage=GetNetMode()==NM_Standalone?1:4;bSettingsOpen=false;SetMenuOpen(true);}
void ADinoPlayerController::ChooseSpecies(int32 Index)
{
    if(GetNetMode()!=NM_Standalone){ServerLobbyAction(0,Index);return;}
    if(OnlinePage>0)return;
    DinoSpecies(Index);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->StartRound();SetMenuOpen(false);
}
void ADinoPlayerController::ServerLobbyAction_Implementation(uint8 Action,int32 Value)
{if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->LobbyAction(this,Action,Value);}
void ADinoPlayerController::ClientLobbyMessage_Implementation(const FString& Message){LobbyStatus=Message;}
void ADinoPlayerController::ClientMatchStarted_Implementation(){MapPins.Empty();LobbyStatus.Empty();OnlinePage=4;bWaitingForRoundStart=true;SetMenuOpen(false);}
void ADinoPlayerController::ServerDisplayName_Implementation(const FString& Name)
{
    FString Safe;for(TCHAR C:Name.Left(32))if(C>=32&&C!=127)Safe.AppendChar(C);
    if(!Safe.IsEmpty())if(auto* PS=GetPlayerState<ADinoPlayerState>())PS->SetPlayerName(Safe);
}
void ADinoPlayerController::OnlineClick(float X,float Y)
{
    auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>();if(!Online)return;
    auto* GS=GetWorld()->GetGameState<ADinoGameState>();auto* PS=GetPlayerState<ADinoPlayerState>();
    auto At=[&](float A,float B,float C,float D){return X>=A&&X<=A+C&&Y>=B&&Y<=B+D;};
    if(GetNetMode()!=NM_Standalone)
    {
        if(At(.08f,.88f,.25f,.065f)){Online->Leave();return;}
        if(At(.36f,.88f,.25f,.065f)){Online->InviteFriends();return;}
        if(At(.65f,.88f,.25f,.065f)){bSettingsOpen=true;return;}
        if(!GS||!PS)return;
        if(GS->bLobby)
        {
            for(int32 I=0;I<3;++I)if(At(.08f+I*.28f,.17f,.25f,.065f)){ServerLobbyAction(0,I);return;}
            if(At(.08f,.26f,.25f,.065f)){ServerLobbyAction(1,-1);return;}
            if(At(.36f,.26f,.25f,.065f)){ServerLobbyAction(1,PS->TeamID==0?1:0);return;}
            if(At(.65f,.26f,.25f,.065f)){ServerLobbyAction(2,!PS->bReady);return;}
            if(At(.08f,.71f,.25f,.065f)){ServerLobbyAction(3,!GS->bTeamMatch);return;}
            if(At(.36f,.71f,.12f,.065f)){ServerLobbyAction(4,GS->MaxParticipants-1);return;}
            if(At(.49f,.71f,.12f,.065f)){ServerLobbyAction(4,GS->MaxParticipants+1);return;}
            if(At(.65f,.71f,.25f,.065f)){ServerLobbyAction(5,!GS->bFillBots);return;}
            if(At(.08f,.79f,.25f,.065f)){ServerLobbyAction(9,!GS->bPerformanceMap);return;}
            if(At(.36f,.79f,.25f,.065f)){ServerLobbyAction(12,0);return;}
            if(At(.65f,.79f,.25f,.065f)){ServerLobbyAction(6,0);return;}
            for(int32 ID=0;ID<GS->MaxParticipants;++ID)
            {
                if(At(.66f,.39f+ID*.028f,.12f,.027f)){ServerLobbyAction(11,ID);return;}
                if(At(.80f,.39f+ID*.028f,.10f,.027f)){ServerLobbyAction(10,ID);return;}
            }
        }
        else
        {
            if(At(.08f,.76f,.25f,.065f)){ServerLobbyAction(7,0);return;}
            if(At(.36f,.76f,.25f,.065f)){if(GS->bRoundOver)ServerLobbyAction(8,0);else SetMenuOpen(false);return;}
        }
        return;
    }
    if(At(.08f,.88f,.25f,.065f)){OnlinePage=OnlinePage==1?0:1;return;}
    if(At(.65f,.88f,.25f,.065f)){Online->SignIn();return;}
    if(OnlinePage==1)
    {
        if(At(.25f,.32f,.5f,.09f))OnlinePage=2;
        if(At(.25f,.47f,.5f,.09f)){OnlinePage=3;SelectedSession=-1;Online->Search();}
    }
    else if(OnlinePage==2)
    {
        if(At(.2f,.28f,.6f,.07f))bHostTeams=!bHostTeams;
        if(At(.2f,.39f,.28f,.07f))HostCapacity=FMath::Max(2,HostCapacity-1);
        if(At(.52f,.39f,.28f,.07f))HostCapacity=FMath::Min(10,HostCapacity+1);
        if(At(.2f,.50f,.6f,.07f))bHostBots=!bHostBots;
        if(At(.2f,.59f,.6f,.055f))bHostPublic=!bHostPublic;
        if(At(.2f,.655f,.6f,.055f))bHostPerformance=!bHostPerformance;
        if(At(.2f,.73f,.6f,.07f))Online->Host(bHostTeams,HostCapacity,bHostBots,bHostPublic,bHostPerformance);
    }
    else if(OnlinePage==3)
    {
        for(int32 I=0;I<FMath::Min(7,Online->Results.Num());++I)if(At(.08f,.26f+I*.065f,.82f,.055f))SelectedSession=I;
        if(At(.08f,.76f,.25f,.065f)){SelectedSession=-1;Online->Search();}
        if(At(.65f,.76f,.25f,.065f))Online->Join(SelectedSession);
    }
}

void ADinoHUD::DrawOnline(ADinoPlayerController* PC)
{
    const float W=Canvas->SizeX,H=Canvas->SizeY;
    const FLinearColor MenuGold(.93f,.72f,.38f),MenuTeal(.32f,.78f,.72f),MenuMuted(.64f,.72f,.69f);
    auto* Online=PC->GetGameInstance()->GetSubsystem<UDinoOnlineSession>();
    auto* GS=GetWorld()->GetGameState<ADinoGameState>();auto* PS=PC->GetPlayerState<ADinoPlayerState>();
    DrawRect(FLinearColor(.009f,.019f,.022f,.97f),0,0,W,H);
    auto Label=[&](const FString& S,float X,float Y,float Size=.85f,FLinearColor C=FLinearColor::White){Text(S,W*X,H*Y,Size,C);};
    auto Button=[&](const FString& S,float X,float Y,float Width=.25f,float Height=.065f,bool Enabled=true)
    {
        float MX=-1,MY=-1;PC->GetMousePosition(MX,MY);bool Hover=MX>W*X&&MX<W*(X+Width)&&MY>H*Y&&MY<H*(Y+Height);
        DrawRect(Enabled?(Hover?FLinearColor(.13f,.30f,.29f):FLinearColor(.07f,.17f,.18f)):FLinearColor(.07f,.09f,.09f),W*X,H*Y,W*Width,H*Height);
        Label(S,X+.012f,Y+Height*.25f,.87f,Enabled?MenuTeal:MenuMuted);
    };
    Label(TEXT("DINO ROYALE / MULTIPLAYER"),.08f,.065f,1.4f,MenuGold);
    if(GetNetMode()!=NM_Standalone)
    {
        if(!GS||!PS){Label(TEXT("Connecting to match..."),.08f,.2f,1.1f);return;}
        Label(FString::Printf(TEXT("%s  /  %d of %d players  /  bots %s"),*GS->MatchName(),GS->PlayerArray.Num(),GS->MaxParticipants,GS->bFillBots?TEXT("ON"):TEXT("OFF")),.08f,.12f,.86f,MenuMuted);
        if(GS->bLobby)
        {
            const TCHAR* Kinds[]={TEXT("1  TYRANNOSAURUS"),TEXT("2  VELOCIRAPTOR"),TEXT("3  TRICERATOPS")};
            for(int32 I=0;I<3;++I)Button(FString(PS->SelectedSpecies==I?TEXT("[X] "):TEXT(""))+Kinds[I],.08f+I*.28f,.17f);
            Button(TEXT("AUTO TEAM"),.08f,.26f,.25f,.065f,GS->bTeamMatch);
            Button(GS->bTeamMatch?FString::Printf(TEXT("TEAM %d / CHANGE"),PS->TeamID+1):TEXT("FREE-FOR-ALL"),.36f,.26f,.25f,.065f,GS->bTeamMatch);
            Button(PS->bReady?TEXT("READY / UNREADY"):TEXT("MARK READY"),.65f,.26f);
            Label(TEXT("PLAYER"),.08f,.35f,.78f,MenuGold);Label(TEXT("DINOSAUR"),.40f,.35f,.78f,MenuGold);
            Label(TEXT("TEAM"),.69f,.35f,.78f,MenuGold);Label(TEXT("READY"),.80f,.35f,.78f,MenuGold);
            for(int32 ID=0;ID<GS->MaxParticipants;++ID)
            {
                const float RowY=.39f+ID*.028f;
                ADinoPlayerState* Human=nullptr;for(auto P:GS->PlayerArray)if(auto* Other=Cast<ADinoPlayerState>(P))if(Other->CombatantID==ID){Human=Other;break;}
                if(Human)
                {
                    Label((Human->bHost?TEXT("HOST  "):TEXT(""))+Human->GetPlayerName().Left(24),.08f,RowY,.75f,Human==PS?MenuTeal:MenuMuted);
                    Label(FSpeciesData::Get(Human->SelectedSpecies).Name,.40f,RowY,.75f);
                    Label(GS->bTeamMatch?(Human->TeamID==0?TEXT("A"):TEXT("B")):TEXT("--"),.69f,RowY,.75f);
                    Label(Human->bHost?TEXT("HOST"):Human->bReady?TEXT("YES"):TEXT("WAIT"),.80f,RowY,.75f);
                }
                else
                {
                    const int32 Team=GS->BotSlotTeams.IsValidIndex(ID)?GS->BotSlotTeams[ID]:-1;
                    const bool Enabled=GS->bFillBots&&Team>=0;
                    Label((Enabled?FString::Printf(TEXT("AI SLOT %d"),ID+1):FString::Printf(TEXT("SLOT %d / EMPTY"),ID+1)),.08f,RowY,.75f,Enabled?MenuTeal:MenuMuted);
                    Label(Enabled?FSpeciesData::Get(ID%3).Name:TEXT("--"),.40f,RowY,.75f);
                    Label(GS->bTeamMatch?(Team==1?TEXT("[ B ]"):TEXT("[ A ]")):TEXT("--"),.69f,RowY,.75f,PS->bHost?MenuTeal:MenuMuted);
                    Label(Enabled?TEXT("[ ON ]"):TEXT("[ OFF ]"),.80f,RowY,.75f,PS->bHost?MenuTeal:MenuMuted);
                }
            }
            Button(TEXT("MODE / CHANGE"),.08f,.71f,.25f,.065f,PS->bHost);
            Button(TEXT("SLOTS -"),.36f,.71f,.12f,.065f,PS->bHost);Button(TEXT("SLOTS +"),.49f,.71f,.12f,.065f,PS->bHost);
            Button(GS->bFillBots?TEXT("BOTS ON / CHANGE"):TEXT("BOTS OFF / CHANGE"),.65f,.71f,.25f,.065f,PS->bHost);
            Button(GS->bPerformanceMap?TEXT("MAP: PERFORMANCE"):TEXT("MAP: STANDARD"),.08f,.79f,.25f,.065f,PS->bHost);
            Button(TEXT("5v5 BOT PRESET"),.36f,.79f,.25f,.065f,PS->bHost);
            Button(TEXT("START MATCH"),.65f,.79f,.25f,.065f,PS->bHost);
        }
        else
        {
            Label(GS->bRoundOver?GS->WinnerName():TEXT("MATCH IN PROGRESS"),.08f,.22f,1.5f,MenuGold);
            Label(TEXT("PLAYER / DINOSAUR"),.08f,.31f,.8f,MenuMuted);
            Label(TEXT("KILLS"),.66f,.31f,.8f,MenuMuted);Label(TEXT("DEATHS"),.74f,.31f,.8f,MenuMuted);Label(TEXT("ASSISTS"),.83f,.31f,.8f,MenuMuted);
            TArray<int32> IDs;GS->Scores.GetKeys(IDs);IDs.Sort([&](int32 A,int32 B){return GS->GetScore(A).Kills>GS->GetScore(B).Kills;});
            float Y=.36f;
            for(int32 ID:IDs)
            {
                auto* D=GS->FindCombatant(ID);if(!D||!GS->IsScoringTarget(D))continue;
                FString Name=TEXT("BOT");for(auto P:GS->PlayerArray)if(auto* Other=Cast<ADinoPlayerState>(P))if(Other->CombatantID==ID)Name=Other->GetPlayerName();
                auto Score=GS->GetScore(ID);Label(Name.Left(22)+TEXT(" / ")+D->Stats().Name,.08f,Y,.78f,ID==PS->CombatantID?MenuTeal:MenuMuted);
                Label(FString::FromInt(Score.Kills),.66f,Y,.85f);Label(FString::FromInt(Score.Deaths),.74f,Y,.85f);Label(FString::FromInt(Score.Assists),.83f,Y,.85f);Y+=.032f;
            }
            Button(TEXT("RETURN TO LOBBY"),.08f,.76f,.25f,.065f,PS->bHost);
            Button(GS->bRoundOver?TEXT("REMATCH"):TEXT("RESUME"),.36f,.76f,.25f,.065f,!GS->bRoundOver||PS->bHost);
            Label(PC->LobbyStatus,.08f,.84f,.78f,MenuGold);
        }
        if(Online)Label(PC->LobbyStatus.IsEmpty()?Online->Status:PC->LobbyStatus,.08f,.96f,.65f,MenuMuted);
        Button(TEXT("LEAVE MATCH"),.08f,.88f);Button(TEXT("INVITE FRIENDS"),.36f,.88f);Button(TEXT("LOCAL SETTINGS / F2"),.65f,.88f);return;
    }
    const int32 Page=PC->OnlinePage;
    Label(Page==2?TEXT("HOST GAME"):Page==3?TEXT("JOIN GAME"):TEXT("PLAY WITH FRIENDS OVER THE INTERNET"),.08f,.17f,1.05f,MenuTeal);
    if(Page==1)
    {
        Button(TEXT("HOST GAME"),.25f,.32f,.5f,.09f);Button(TEXT("JOIN GAME"),.25f,.47f,.5f,.09f);
        Label(TEXT("Sign in to Epic, then host or find a session."),.25f,.62f,.92f,MenuMuted);
    }
    else if(Page==2)
    {
        Button(PC->bHostTeams?TEXT("MODE: TEAM BATTLE"):TEXT("MODE: FREE-FOR-ALL"),.2f,.28f,.6f,.07f);
        Button(FString::Printf(TEXT("PARTICIPANTS: %d    -"),PC->HostCapacity),.2f,.39f,.28f,.07f);Button(TEXT("+    PARTICIPANTS"),.52f,.39f,.28f,.07f);
        Button(PC->bHostBots?TEXT("FILL EMPTY SLOTS WITH BOTS: ON"):TEXT("FILL EMPTY SLOTS WITH BOTS: OFF"),.2f,.50f,.6f,.07f);
        Button(PC->bHostPublic?TEXT("VISIBILITY: PUBLIC / SEARCHABLE"):TEXT("VISIBILITY: INVITE ONLY"),.2f,.59f,.6f,.055f);
        Button(PC->bHostPerformance?TEXT("MAP: SUNGRASS PLAINS - PERFORMANCE"):TEXT("MAP: SUNGRASS PLAINS - STANDARD"),.2f,.655f,.6f,.055f);
        Button(TEXT("CREATE LOBBY"),.2f,.73f,.6f,.07f,Online&&!Online->bBusy);
    }
    else if(Page==3&&Online)
    {
        for(int32 I=0;I<FMath::Min(7,Online->Results.Num());++I)Button((PC->SelectedSession==I?TEXT("> "):TEXT(""))+Online->ResultLabel(I),.08f,.26f+I*.065f,.82f,.055f);
        Button(TEXT("REFRESH"),.08f,.76f);Button(TEXT("JOIN SELECTED"),.65f,.76f,.25f,.065f,PC->SelectedSession>=0&&!Online->bBusy);
    }
    if(Online)Label(Online->Status,.08f,.83f,.80f,MenuGold);
    Button(TEXT("BACK"),.08f,.88f);Button(Online&&Online->IsSignedIn()?TEXT("SIGNED IN"):TEXT("SIGN IN TO EPIC"),.65f,.88f);
}
