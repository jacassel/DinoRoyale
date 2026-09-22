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
        if(At(.08,.88,.25,.065)){Online->Leave();return;}
        if(At(.36,.88,.25,.065)){Online->InviteFriends();return;}
        if(At(.65,.88,.25,.065)){bSettingsOpen=true;return;}
        if(!GS||!PS)return;
        if(GS->bLobby)
        {
            for(int32 I=0;I<3;++I)if(At(.08+I*.28,.17,.25,.065)){ServerLobbyAction(0,I);return;}
            if(At(.08,.26,.25,.065)){ServerLobbyAction(1,-1);return;}
            if(At(.36,.26,.25,.065)){ServerLobbyAction(1,PS->TeamID==0?1:0);return;}
            if(At(.65,.26,.25,.065)){ServerLobbyAction(2,!PS->bReady);return;}
            if(At(.08,.71,.25,.065)){ServerLobbyAction(3,!GS->bTeamMatch);return;}
            if(At(.36,.71,.12,.065)){ServerLobbyAction(4,GS->MaxParticipants-1);return;}
            if(At(.49,.71,.12,.065)){ServerLobbyAction(4,GS->MaxParticipants+1);return;}
            if(At(.65,.71,.25,.065)){ServerLobbyAction(5,!GS->bFillBots);return;}
            if(At(.65,.79,.25,.065)){ServerLobbyAction(6,0);return;}
        }
        else
        {
            if(At(.08,.76,.25,.065)){ServerLobbyAction(7,0);return;}
            if(At(.36,.76,.25,.065)){if(GS->bRoundOver)ServerLobbyAction(8,0);else SetMenuOpen(false);return;}
        }
        return;
    }
    if(At(.08,.88,.25,.065)){OnlinePage=OnlinePage==1?0:1;return;}
    if(At(.65,.88,.25,.065)){Online->SignIn();return;}
    if(OnlinePage==1)
    {
        if(At(.25,.32,.5,.09))OnlinePage=2;
        if(At(.25,.47,.5,.09)){OnlinePage=3;SelectedSession=-1;Online->Search();}
    }
    else if(OnlinePage==2)
    {
        if(At(.2,.28,.6,.07))bHostTeams=!bHostTeams;
        if(At(.2,.39,.28,.07))HostCapacity=FMath::Max(2,HostCapacity-1);
        if(At(.52,.39,.28,.07))HostCapacity=FMath::Min(10,HostCapacity+1);
        if(At(.2,.50,.6,.07))bHostBots=!bHostBots;
        if(At(.2,.61,.6,.07))bHostPublic=!bHostPublic;
        if(At(.2,.73,.6,.07))Online->Host(bHostTeams,HostCapacity,bHostBots,bHostPublic);
    }
    else if(OnlinePage==3)
    {
        for(int32 I=0;I<FMath::Min(7,Online->Results.Num());++I)if(At(.08,.26+I*.065,.82,.055))SelectedSession=I;
        if(At(.08,.76,.25,.065)){SelectedSession=-1;Online->Search();}
        if(At(.65,.76,.25,.065))Online->Join(SelectedSession);
    }
}

void ADinoHUD::DrawOnline(ADinoPlayerController* PC)
{
    const float W=Canvas->SizeX,H=Canvas->SizeY;
    const FLinearColor Gold(.93f,.72f,.38f),Teal(.32f,.78f,.72f),Muted(.64f,.72f,.69f);
    auto* Online=PC->GetGameInstance()->GetSubsystem<UDinoOnlineSession>();
    auto* GS=GetWorld()->GetGameState<ADinoGameState>();auto* PS=PC->GetPlayerState<ADinoPlayerState>();
    DrawRect(FLinearColor(.009f,.019f,.022f,.97f),0,0,W,H);
    auto Label=[&](const FString& S,float X,float Y,float Size=.85f,FLinearColor C=FLinearColor::White){Text(S,W*X,H*Y,Size,C);};
    auto Button=[&](const FString& S,float X,float Y,float Width=.25f,float Height=.065f,bool Enabled=true)
    {
        float MX=-1,MY=-1;PC->GetMousePosition(MX,MY);bool Hover=MX>W*X&&MX<W*(X+Width)&&MY>H*Y&&MY<H*(Y+Height);
        DrawRect(Enabled?(Hover?FLinearColor(.13f,.30f,.29f):FLinearColor(.07f,.17f,.18f)):FLinearColor(.07f,.09f,.09f),W*X,H*Y,W*Width,H*Height);
        Label(S,X+.012f,Y+Height*.25f,.87f,Enabled?Teal:Muted);
    };
    Label(TEXT("DINOSAUR BATTLE / MULTIPLAYER"),.08,.065,1.4f,Gold);
    if(GetNetMode()!=NM_Standalone)
    {
        if(!GS||!PS){Label(TEXT("Connecting to match..."),.08,.2,1.1f);return;}
        Label(FString::Printf(TEXT("%s  /  %d of %d players  /  bots %s"),*GS->MatchName(),GS->PlayerArray.Num(),GS->MaxParticipants,GS->bFillBots?TEXT("ON"):TEXT("OFF")),.08,.12,.86f,Muted);
        if(GS->bLobby)
        {
            const TCHAR* Kinds[]={TEXT("1  TYRANNOSAURUS"),TEXT("2  VELOCIRAPTOR"),TEXT("3  TRICERATOPS")};
            for(int32 I=0;I<3;++I)Button(FString(PS->SelectedSpecies==I?TEXT("[X] "):TEXT(""))+Kinds[I],.08+I*.28,.17);
            Button(TEXT("AUTO TEAM"),.08,.26,.25,.065,GS->bTeamMatch);
            Button(GS->bTeamMatch?FString::Printf(TEXT("TEAM %d / CHANGE"),PS->TeamID+1):TEXT("FREE-FOR-ALL"),.36,.26,.25,.065,GS->bTeamMatch);
            Button(PS->bReady?TEXT("READY / UNREADY"):TEXT("MARK READY"),.65,.26);
            Label(TEXT("PLAYER                          DINOSAUR                 TEAM     READY"),.08,.35,.78f,Gold);
            float Y=.39;
            for(auto P:GS->PlayerArray)if(auto* Other=Cast<ADinoPlayerState>(P))
            {
                Label((Other->bHost?TEXT("HOST  "):TEXT(""))+Other->GetPlayerName().Left(24),.08,Y,.78f,Other==PS?Teal:Muted);
                Label(FSpeciesData::Get(Other->SelectedSpecies).Name,.40,Y,.78f);
                Label(GS->bTeamMatch?FString::FromInt(Other->TeamID+1):TEXT("--"),.69,Y,.78f);
                Label(Other->bHost?TEXT("HOST"):Other->bReady?TEXT("YES"):TEXT("WAIT"),.80,Y,.78f);Y+=.028f;
            }
            Button(TEXT("MODE / CHANGE"),.08,.71,.25,.065,PS->bHost);
            Button(TEXT("SLOTS -"),.36,.71,.12,.065,PS->bHost);Button(TEXT("SLOTS +"),.49,.71,.12,.065,PS->bHost);
            Button(GS->bFillBots?TEXT("BOTS ON / CHANGE"):TEXT("BOTS OFF / CHANGE"),.65,.71,.25,.065,PS->bHost);
            Label(PC->LobbyStatus.IsEmpty()?TEXT("The host starts when guests are ready."):PC->LobbyStatus,.08,.81,.8f,Gold);
            Button(TEXT("START MATCH"),.65,.79,.25,.065,PS->bHost);
        }
        else
        {
            Label(GS->bRoundOver?GS->WinnerName():TEXT("MATCH IN PROGRESS"),.08,.22,1.5f,Gold);
            Label(TEXT("PLAYER / DINOSAUR                           KILLS   DEATHS   ASSISTS"),.08,.31,.8f,Muted);
            TArray<int32> IDs;GS->Scores.GetKeys(IDs);IDs.Sort([&](int32 A,int32 B){return GS->GetScore(A).Kills>GS->GetScore(B).Kills;});
            float Y=.36;
            for(int32 ID:IDs)
            {
                auto* D=GS->FindCombatant(ID);if(!D||!GS->IsScoringTarget(D))continue;
                FString Name=TEXT("BOT");for(auto P:GS->PlayerArray)if(auto* Other=Cast<ADinoPlayerState>(P))if(Other->CombatantID==ID)Name=Other->GetPlayerName();
                auto Score=GS->GetScore(ID);Label(Name.Left(22)+TEXT(" / ")+D->Stats().Name,.08,Y,.78f,ID==PS->CombatantID?Teal:Muted);
                Label(FString::Printf(TEXT("%d       %d       %d"),Score.Kills,Score.Deaths,Score.Assists),.66,Y,.85f);Y+=.032f;
            }
            Button(TEXT("RETURN TO LOBBY"),.08,.76,.25,.065,PS->bHost);
            Button(GS->bRoundOver?TEXT("REMATCH"):TEXT("RESUME"),.36,.76,.25,.065,!GS->bRoundOver||PS->bHost);
            Label(PC->LobbyStatus,.08,.84,.78f,Gold);
        }
        Button(TEXT("LEAVE MATCH"),.08,.88);Button(TEXT("INVITE FRIENDS"),.36,.88);Button(TEXT("LOCAL SETTINGS / F2"),.65,.88);return;
    }
    const int32 Page=PC->OnlinePage;
    Label(Page==2?TEXT("HOST GAME"):Page==3?TEXT("JOIN GAME"):TEXT("PLAY WITH FRIENDS OVER THE INTERNET"),.08,.17,1.05f,Teal);
    if(Page==1)
    {
        Button(TEXT("HOST GAME"),.25,.32,.5,.09);Button(TEXT("JOIN GAME"),.25,.47,.5,.09);
        Label(TEXT("Sign in to Epic, then host or find a session."),.25,.62,.92f,Muted);
    }
    else if(Page==2)
    {
        Button(PC->bHostTeams?TEXT("MODE: TEAM BATTLE"):TEXT("MODE: FREE-FOR-ALL"),.2,.28,.6,.07);
        Button(FString::Printf(TEXT("PARTICIPANTS: %d    -"),PC->HostCapacity),.2,.39,.28,.07);Button(TEXT("+    PARTICIPANTS"),.52,.39,.28,.07);
        Button(PC->bHostBots?TEXT("FILL EMPTY SLOTS WITH BOTS: ON"):TEXT("FILL EMPTY SLOTS WITH BOTS: OFF"),.2,.50,.6,.07);
        Button(PC->bHostPublic?TEXT("VISIBILITY: PUBLIC / SEARCHABLE"):TEXT("VISIBILITY: INVITE ONLY"),.2,.61,.6,.07);
        Button(TEXT("CREATE LOBBY"),.2,.73,.6,.07,Online&&!Online->bBusy);
    }
    else if(Page==3&&Online)
    {
        for(int32 I=0;I<FMath::Min(7,Online->Results.Num());++I)Button((PC->SelectedSession==I?TEXT("> "):TEXT(""))+Online->ResultLabel(I),.08,.26+I*.065,.82,.055);
        Button(TEXT("REFRESH"),.08,.76);Button(TEXT("JOIN SELECTED"),.65,.76,.25,.065,PC->SelectedSession>=0&&!Online->bBusy);
    }
    if(Online)Label(Online->Status,.08,.83,.80f,Gold);
    Button(TEXT("BACK"),.08,.88);Button(Online&&Online->IsSignedIn()?TEXT("SIGNED IN"):TEXT("SIGN IN TO EPIC"),.65,.88);
}
