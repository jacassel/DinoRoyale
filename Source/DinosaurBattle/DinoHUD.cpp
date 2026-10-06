#include "DinoHUD.h"
#include "DinosaurCharacter.h"
#include "DinoPlayerController.h"
#include "DinoGameState.h"
#include "DinoPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "LostValleyWorld.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "HungerComponent.h"
#include "CombatComponent.h"
#include "FoodSystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
namespace {const FLinearColor Gold(.93f,.72f,.38f),Teal(.32f,.78f,.72f),Red(.88f,.29f,.20f),Muted(.64f,.72f,.69f);}
void ADinoHUD::Text(const FString& M,float X,float Y,float S,FLinearColor C)
{
    if(!DisplayFont)DisplayFont=LoadObject<UFont>(nullptr,TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
    UFont* Font=DisplayFont?DisplayFont:GEngine->GetMediumFont();
    float Ratio=float(GEngine->GetMediumFont()->GetMaxCharHeight())/FMath::Max(1,Font->GetMaxCharHeight());
    DrawText(M,C,X,Y,Font,S*Scale*1.6f*Ratio);
}
void ADinoHUD::Panel(float X,float Y,float W,float H,float A){DrawRect(FLinearColor(.018f,.033f,.035f,A),X,Y,W,H);}
void ADinoHUD::Bar(float X,float Y,float W,float H,float F,FLinearColor C){DrawRect(FLinearColor(.10f,.14f,.14f,.95f),X,Y,W,H);DrawRect(C,X,Y,W*FMath::Clamp(F,0.f,1.f),H);}
void ADinoHUD::DrawHUD()
{
    Super::DrawHUD();if(!Canvas)return;auto* D=Cast<ADinosaurCharacter>(GetOwningPawn());auto* PC=Cast<ADinoPlayerController>(PlayerOwner);if(!PC)return;
    Scale=FMath::Clamp(Canvas->SizeY/900.f,.45f,1.5f);float S=Scale,W=Canvas->SizeX,H=Canvas->SizeY;
    if(!bMapLoaded){WorldMap=LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/T_ValleyMap.T_ValleyMap"));bMapLoaded=true;}
    if(PC->bSelectionOpen&&!PC->bSettingsOpen&&(PC->OnlinePage>0||GetNetMode()!=NM_Standalone)){DrawOnline(PC);return;}
    if(!D)return;
    if(PC->bSelectionOpen){DrawMenu(D,PC);return;}
    if(auto* GM=GetWorld()->GetGameState<ADinoGameState>())
    {
        auto Score=GM->GetScore(D->CombatantID);
        Panel(24*S,H-188*S,246*S,75*S);
        Text(TEXT("KILLS     DEATHS     ASSISTS"),40*S,H-176*S,.69f,Muted);
        Text(FString::Printf(TEXT("%d          %d          %d"),Score.Kills,Score.Deaths,Score.Assists),43*S,H-151*S,1.35f,Gold);
        FString Goal=GM->bTeamMatch?FString::Printf(TEXT("TEAM BATTLE   YOUR TEAM %d - %d RIVALS   /   GOAL %d"),GM->TeamPoints(FMath::Clamp(D->TeamID,0,1)),GM->TeamPoints(1-FMath::Clamp(D->TeamID,0,1)),GM->TeamKillGoal):FString::Printf(TEXT("FREE-FOR-ALL   %d / %d POINTS"),Score.SoloPoints(),GM->SoloKillGoal);
        Text(Goal,W*.5f-160*S+S,79*S+S,.73f,FLinearColor::White);
        Text(Goal,W*.5f-160*S,79*S,.73f,FLinearColor::Black);
        if(!GM->bTeamMatch)
        {
            const auto IDs=GM->LeaderboardIDs();FString Lead=TEXT("LEADER: NO SCORES YET");
            if(!IDs.IsEmpty())
            {
                const int32 Kills=GM->GetScore(IDs[0]).SoloPoints();int32 Tied=0;
                for(int32 ID:IDs)if(GM->GetScore(ID).SoloPoints()==Kills)++Tied;
                Lead=Kills==0?TEXT("LEADER: ALL TIED AT 0"):FString::Printf(TEXT("LEADER: %s - %d POINTS%s"),*GM->CombatantName(IDs[0],D->CombatantID),Kills,Tied>1?*FString::Printf(TEXT(" (%d TIED)"),Tied):TEXT(""));
            }
            Text(Lead,W*.5f-160*S+S,103*S+S,.72f,FLinearColor::White);
            Text(Lead,W*.5f-160*S,103*S,.72f,FLinearColor::Black);
        }
    }
    if(auto* GS=GetWorld()->GetGameState<ADinoGameState>())
        Text(GS->bTeamMatch?FString::Printf(TEXT("3 TEAM ASSISTS = 1 POINT  /  %d OF 3"),GS->TeamAssists[FMath::Clamp(D->TeamID,0,1)]%3):TEXT("2 ASSISTS = 1 POINT   /   P LEADERBOARD"),W*.5f-160*S,127*S,.65f,FLinearColor::Black);
    Panel(24*S,24*S,386*S,230*S);
    Text(TEXT("DINO ROYALE / VERSION 0.5"),42*S,37*S,.95f,Gold);
    Text(D->Stats().Name,42*S,64*S,1.32f);
    FLinearColor HealthColor=D->Health->Fraction()<.25f?Red:D->Health->Fraction()<.5f?Gold:Teal;
    Bar(42*S,99*S,350*S,12*S,D->Health->Fraction(),HealthColor);
    FString Status=D->bDead?FString::Printf(TEXT("RESPAWN IN %.0f"),FMath::Max(0.f,D->RespawnDelay-D->DeathTime)):D->Combat->bBracing?TEXT("BRACED - FRONT PROTECTED"):D->Food->bEating?TEXT("FEEDING"):D->Health->Fraction()<.25f?TEXT("CRITICAL - NO CHARGE"):D->Health->Fraction()<.5f?TEXT("INJURED - SLOWED"):TEXT("HEALTHY");
    Text(FString::Printf(TEXT("%.0f / %.0f   %s"),D->Health->Current,D->Health->Maximum,*Status),42*S,124*S,.82f,HealthColor);
    const auto StaminaColor=D->Stamina->bExhausted?Red:D->Stamina->Fraction()<.3f?Gold:Teal;
    Bar(42*S,152*S,350*S,9*S,D->Stamina->Fraction(),StaminaColor);
    Text(FString::Printf(TEXT("STAMINA %.0f / %.0f   %s"),D->Stamina->Current,D->Stamina->Maximum,D->Stamina->bExhausted?TEXT("EXHAUSTED"):D->bSprinting?TEXT("SPRINTING"):TEXT("SHIFT TO SPRINT")),42*S,169*S,.73f,StaminaColor);
    Text(FString::Printf(TEXT("COMBO %d / 3%s"),D->Combat->ComboCount,D->Combat->ComboCount==3&&D->Combat->IsBusy()?TEXT(" - RECOVERING"):TEXT("")),42*S,190*S,.68f,Muted);
    const float Hunger=D->Hunger->Fraction();const auto HungerColor=Hunger<=.2f?Red:Hunger<.7f?Gold:Teal;
    Bar(42*S,213*S,350*S,9*S,Hunger,HungerColor);
    Text(FString::Printf(TEXT("HUNGER %.0f%%   %s"),Hunger*100,Hunger<=.1f?TEXT("STARVING - EAT"):Hunger<=.2f?TEXT("NO STAMINA REGEN"):Hunger<.4f?TEXT("NO HEALTH REGEN"):Hunger<.7f?TEXT("HUNGRY"):TEXT("WELL FED")),42*S,230*S,.71f,HungerColor);
    Text(ALostValleyWorld::RegionName(D->GetActorLocation()),W*.5f-150*S+S,30*S+S,.98f,FLinearColor::White);
    Text(ALostValleyWorld::RegionName(D->GetActorLocation()),W*.5f-150*S,30*S,.98f,FLinearColor::Black);
    Text(D->bSwimming?TEXT("SWIMMING - SPACE TO SURGE"):D->bInWater?TEXT("WADING - MOVEMENT SLOWED"):TEXT("LOST VALLEY"),W*.5f-80*S,51*S,.70f,FLinearColor::Black);
    if(FSpeciesData::IsPack(D->Species))
    {
        int32 Alive=0,Close=0;
        for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(*It!=D&&It->Species==D->Species&&!It->bDead&&!D->IsEnemy(*It)&&(GetNetMode()==NM_Standalone||(It->bPackFollower&&It->PackLeaderID==D->CombatantID))){++Alive;if(FVector::Dist2D(D->GetActorLocation(),It->GetActorLocation())<5000)++Close;}
        Text(FString::Printf(TEXT("PACK LEADER   %d nearby / %d alive"),Close,Alive),42*S,268*S,.9f,Teal);
    }
    float Since=GetWorld()->GetTimeSeconds()-D->Health->LastDamageTime;
    if(!D->bDead&&D->Health->Fraction()<1&&!D->Food->bEating)
        Text(Since<D->Stats().RegenDelay?FString::Printf(TEXT("Regeneration in %.1fs"),D->Stats().RegenDelay-Since):D->Hunger->HealthRegenFactor()<=0?TEXT("Eat to restore health regeneration"):TEXT("Regenerating - rate depends on hunger"),42*S,293*S,.8f,Muted);
    if(D->Health->HitFlash>0)
    {
        FLinearColor Flash(.75f,.12f,.08f,D->Health->HitFlash*.8f);
        DrawRect(Flash,0,0,W,15*S);DrawRect(Flash,0,H-15*S,W,15*S);DrawRect(Flash,0,0,15*S,H);DrawRect(Flash,W-15*S,0,15*S,H);
    }
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* O=*It;if(O==D||O->bDead||!D->CanSeeDinosaur(O)||FVector::DistSquared(D->GetActorLocation(),O->GetActorLocation())>FMath::Square(6000.f))continue;
        const FVector Anchor=O->GetMesh()->DoesSocketExist(TEXT("head"))?O->GetMesh()->GetSocketLocation(TEXT("head"))+FVector(0,0,110):O->GetActorLocation()+FVector(0,0,O->Stats().HalfHeight+80);
        FVector2D P;if(PlayerOwner->ProjectWorldLocationToScreen(Anchor,P)&&P.X>0&&P.X<W&&P.Y>0&&P.Y<H)
        {
            FLinearColor C=D->IsEnemy(O)?Red:Teal;Bar(P.X-45*S,P.Y,90*S,5*S,O->Health->Fraction(),C);
            FString Name=O->Stats().Name;
            if(FSpeciesData::IsPack(O->Species))if(auto* GM=GetWorld()->GetGameState<ADinoGameState>())Name=GM->IsScoringTarget(O)?(O->Species==6?TEXT("PACHY LEADER"):TEXT("RAPTOR LEADER")):TEXT("Pack follower");
            if(auto* GS=GetWorld()->GetGameState<ADinoGameState>())for(auto Player:GS->PlayerArray)
                if(auto* PS=Cast<ADinoPlayerState>(Player))if(PS->CombatantID==O->CombatantID){Name=PS->GetPlayerName();break;}
            if(PC->bShowNameTags&&O->bMajor){Text(Name,P.X-45*S+S,P.Y-19*S+S,.64f,FLinearColor::Black);Text(Name,P.X-45*S,P.Y-19*S,.64f,C);}
        }
    }
    if(D->Combat->bCharging)
    {
        Panel(W*.5f-180*S,H-173*S,360*S,58*S);
        Text(D->Combat->ChargeFraction()>.97f?TEXT("FULL CHARGE - RELEASE RMB"):TEXT("CHARGING - RELEASE RMB"),W*.5f-128*S,H-162*S,.95f,Gold);
        Bar(W*.5f-158*S,H-136*S,316*S,9*S,D->Combat->ChargeFraction(),Gold);
    }
    else if(!D->bDead&&D->Food->FindFood(D->Stats().AttackRange+260))
    {
        Panel(W*.5f-205*S,H-176*S,410*S,66*S);
        Text(D->Food->bEating?TEXT("FEEDING  +HEALTH / STAMINA / HUNGER"):D->Stats().bHerbivore?(D->Species==5?TEXT("HOLD F - BROWSE TREE"):TEXT("HOLD F - EAT SHRUB")):TEXT("HOLD F - FEED ON CARCASS"),W*.5f-185*S,H-164*S,.76f,Teal);
        AActor* Meal=D->Food->FindFood(D->Stats().AttackRange+260);float Remaining=0;
        if(auto* Plant=Cast<AFoodPlant>(Meal))Remaining=Plant->Nutrition;else if(auto* Corpse=Cast<ADinosaurCarcass>(Meal))Remaining=Corpse->Nutrition;
        Text(FString::Printf(TEXT("%.0f food remaining"),Remaining),W*.5f-185*S,H-138*S,.7f,Muted);
    }
    if(PC->bShowHelp)
    {
        Panel(24*S,H-99*S,W-48*S,85*S,.82f);
        Text(TEXT("WASD Move   SHIFT Sprint   MOUSE Look   SPACE Jump   Q / E Pivot   CTRL Brace"),42*S,H-84*S,.94f);
        Text(TEXT("LMB  Quick attack     HOLD / RELEASE RMB  Heavy attack     HOLD F  Eat"),42*S,H-60*S,.88f,Muted);
        Text(GetNetMode()==NM_Standalone?TEXT("1-6  Species     M  Map     P  Leaderboard     H  Help     ESC  Pause"):TEXT("M  Map     P  Leaderboard     H  Help     ESC  Multiplayer menu"),42*S,H-38*S,.78f,Gold);
    }
    else Text(GetNetMode()==NM_Standalone?TEXT("H  Help     P  Leaderboard     M  Map     ESC  Pause"):TEXT("H  Help     P  Leaderboard     M  Map     ESC  Menu"),30*S,H-32*S,.8f,Muted);
    DrawWorldMap(D,PC->bMapOpen);
    if(PC->bLeaderboardOpen)if(auto* GS=GetWorld()->GetGameState<ADinoGameState>())if(!GS->bTeamMatch)
    {
        const float X=W*.22f,Y=H*.22f,Width=W*.56f;
        const auto IDs=GS->LeaderboardIDs();Panel(X,Y,Width,120*S+IDs.Num()*32*S,.96f);
        Text(TEXT("FREE-FOR-ALL LEADERBOARD    P CLOSE"),X+20*S,Y+18*S,.95f,Gold);
        Text(TEXT("PLAYER / DINOSAUR"),X+20*S,Y+58*S,.72f,Muted);
        Text(TEXT("PTS   K   D   A"),X+Width-140*S,Y+58*S,.72f,Muted);
        for(int32 Rank=0;Rank<IDs.Num();++Rank)
        {
            const int32 ID=IDs[Rank];const auto Score=GS->GetScore(ID);const auto C=ID==D->CombatantID?Teal:FLinearColor::White;
            Text(FString::Printf(TEXT("%d. %s"),Rank+1,*GS->CombatantName(ID,D->CombatantID)),X+20*S,Y+(88+Rank*32)*S,.76f,C);
            Text(FString::Printf(TEXT("%d    %d   %d   %d"),Score.SoloPoints(),Score.Kills,Score.Deaths,Score.Assists),X+Width-140*S,Y+(88+Rank*32)*S,.76f,C);
        }
    }
}
void ADinoHUD::DrawMenu(ADinosaurCharacter* D,ADinoPlayerController* PC)
{
    float W=Canvas->SizeX,H=Canvas->SizeY,S=Scale;Panel(0,0,W,H,.93f);
    Text(TEXT("DINO ROYALE"),W*.105f,H*.11f,2.7f,Gold);
    Text(GetNetMode()==NM_Standalone?TEXT("LOST VALLEY  /  OFFLINE PLAY  /  VERSION 0.5"):TEXT("MULTIPLAYER  /  LOCAL SETTINGS  /  VERSION 0.5"),W*.108f,H*.19f,.75f,Muted);
    auto* GM=GetWorld()->GetGameState<ADinoGameState>();
    if(GM&&GM->bRoundOver&&!PC->bSettingsOpen)
    {
        Text(GM->WinnerName(),W*.20f,H*.27f,1.7f,Gold);
        Text(GM->bTeamMatch?TEXT("TEAM RESULTS"):FString::Printf(TEXT("SOLO RESULTS / FIRST TO %d POINTS"),GM->SoloKillGoal),W*.20f,H*.235f,.78f,Teal);
        Text(TEXT("PLAYER / DINOSAUR"),W*.20f,H*.34f,.78f,Muted);
        Text(TEXT("POINTS / KILLS"),W*.56f,H*.34f,.72f,Muted);Text(TEXT("DEATHS"),W*.67f,H*.34f,.72f,Muted);Text(TEXT("ASSISTS"),W*.76f,H*.34f,.72f,Muted);
        TArray<int32> IDs;GM->Scores.GetKeys(IDs);IDs.Sort([&](int32 A,int32 B){auto SA=GM->GetScore(A),SB=GM->GetScore(B);return !GM->bTeamMatch&&SA.SoloPoints()!=SB.SoloPoints()?SA.SoloPoints()>SB.SoloPoints():SA.Kills!=SB.Kills?SA.Kills>SB.Kills:SA.Deaths!=SB.Deaths?SA.Deaths<SB.Deaths:A<B;});
        float Y=H*.39f;
        for(int32 ID:IDs)
        {
            auto* Who=GM->FindCombatant(ID);if(!Who)continue;auto Score=GM->GetScore(ID);FLinearColor C=ID==D->CombatantID?Teal:Muted;
            FString Name=(ID==D->CombatantID?TEXT("YOU / "):FString::Printf(TEXT("AI %d / "),ID))+Who->Stats().Name;
            if(FSpeciesData::IsPack(Who->Species)&&!GM->IsScoringTarget(Who))Name+=TEXT(" follower");
            Text(Name,W*.20f,Y,.76f,C);Text(GM->bTeamMatch?FString::FromInt(Score.Kills):FString::Printf(TEXT("%d / %d"),Score.SoloPoints(),Score.Kills),W*.59f,Y,.84f,C);Text(FString::FromInt(Score.Deaths),W*.67f,Y,.84f,C);Text(FString::FromInt(Score.Assists),W*.76f,Y,.84f,C);Y+=27*S;
        }
        Text(TEXT("ENTER / ESC  Play again     1-6  Change dinosaur     F3  Change mode"),W*.20f,H*.85f,.78f,Gold);
        return;
    }
    if(GetNetMode()==NM_Standalone)
    {
        Text(TEXT("F4  MULTIPLAYER"),W*.76f,H*.245f,.82f,Gold);
        if(GM)Text(TEXT("F3  ")+GM->MatchName(),W*.108f,H*.245f,.88f,Teal);
    }
    Text(PC->bSettingsOpen?TEXT("F2  BACK"):TEXT("F2  SETTINGS"),W*.76f,H*.13f,.95f,Teal);
    if(PC->bSettingsOpen)
    {
        Panel(W*.18f,H*.28f,W*.64f,H*.46f,1);
        Text(TEXT("SETTINGS"),W*.22f,H*.30f,1.4f,Gold);
        Text(TEXT("BLOOD EFFECTS"),W*.22f,H*.37f,1.15f);
        Text(PC->bBloodEnabled?TEXT("[ B ]   ON"):TEXT("[ B ]   OFF"),W*.64f,H*.37f,1.15f,PC->bBloodEnabled?Red:Teal);
        Text(TEXT("Optional blood particles when a dinosaur or prey is hit."),W*.22f,H*.425f,.84f,Muted);
        Text(FString::Printf(TEXT("MOUSE SENSITIVITY     %.1f"),D->MouseSensitivity),W*.22f,H*.525f,1.08f);
        Text(TEXT("[-]       [+]"),W*.65f,H*.525f,1.15f,Gold);
        Text(TEXT("SHOW NAME TAGS"),W*.22f,H*.61f,1.08f);
        Text(PC->bShowNameTags?TEXT("[ N ]   ON"):TEXT("[ N ]   OFF"),W*.64f,H*.61f,1.08f,Teal);
        Text(TEXT("Settings save automatically on this computer."),W*.22f,H*.69f,.8f,Muted);
        Text(TEXT("Q / E Pivot     Ctrl Brace     Shift Sprint     F Eat"),W*.22f,H*.735f,.72f,Muted);
        Text((GetNetMode()==NM_Standalone?TEXT("ESC / F2  Back to dinosaur selection      ENTER  Resume"):TEXT("ESC / F2  Back to multiplayer")),W*.22f,H*.78f,.95f,Teal);
        return;
    }
    const float Top=H*.28f,CardW=W*.26f,CardH=H*.225f,Gap=W*.025f,RowGap=H*.016f,Left=(W-3*CardW-2*Gap)*.5f;
    if(Portraits.Num()!=6){Portraits.SetNumZeroed(6);for(int32 I=0;I<6;++I){const FString Kind=FSpeciesData::Get(FSpeciesData::PlayableID(I)).AssetName;Portraits[I]=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/UI/T_%sPortrait.T_%sPortrait"),*Kind,*Kind));}}
    const TCHAR* Names[]={TEXT("T-REX"),TEXT("VELOCIRAPTOR"),TEXT("TRICERATOPS"),TEXT("ANKYLOSAURUS"),TEXT("BRACHIOSAURUS"),TEXT("PACHYCEPHALOSAURUS")};
    const TCHAR* Roles[]={TEXT("APEX PREDATOR"),TEXT("PACK HUNTER"),TEXT("FRONTLINE DEFENDER"),TEXT("ARMORED TANK"),TEXT("COLOSSUS"),TEXT("PACK CHARGER")};
    const TCHAR* Ability[]={TEXT("Bite / committed lunge"),TEXT("Fast slash / allied pack"),TEXT("Horn charge / frontal brace"),TEXT("Rear club / armored brace"),TEXT("Stomp / holds nearby space"),TEXT("Headbutt / momentum charge")};
    float MX=-1,MY=-1;PC->GetMousePosition(MX,MY);
    for(int32 I=0;I<6;++I)
    {
        const int32 ID=FSpeciesData::PlayableID(I);const auto& Stats=FSpeciesData::Get(ID);
        const float X=Left+(I%3)*(CardW+Gap),Y=Top+(I/3)*(CardH+RowGap);
        const bool Selected=D->Species==ID,Hover=MX>X&&MX<X+CardW&&MY>Y&&MY<Y+CardH;
        const FLinearColor Accent=Selected?Gold:Teal;
        DrawRect(Selected?FLinearColor(.16f,.19f,.12f):Hover?FLinearColor(.10f,.17f,.17f):FLinearColor(.045f,.075f,.085f),X,Y,CardW,CardH);
        DrawRect(Accent,X,Y,CardW,(Selected?4:2)*S);
        Text(Names[I],X+12*S,Y+9*S,I==5?.77f:.90f);Text(Roles[I],X+12*S,Y+31*S,.56f,Accent);
        Text(FString::Printf(TEXT("%d%s"),I+1,Selected?TEXT("  SELECTED"):TEXT("")),X+CardW-95*S,Y+31*S,.53f,Accent);
        const float ArtW=CardW*.55f,ArtH=CardH*.61f;
        if(Portraits[I]){const float Fit=FMath::Min(ArtW/Portraits[I]->GetSizeX(),ArtH/Portraits[I]->GetSizeY());const float PW=Portraits[I]->GetSizeX()*Fit,PH=Portraits[I]->GetSizeY()*Fit;DrawTexture(Portraits[I],X+5*S+(ArtW-PW)*.5f,Y+50*S+(ArtH-PH)*.5f,PW,PH,0,0,1,1,FLinearColor::White,BLEND_Translucent);}
        const TCHAR* Labels[]={TEXT("HEALTH"),TEXT("ATTACK"),TEXT("DEFENSE"),TEXT("SPEED"),TEXT("STAMINA")};
        const float Ratings[]={Stats.MaxHealth/3100,Stats.Damage/240,(1-Stats.BraceMultiplier)*(.65f+.35f*(1-Stats.ArmorMultiplier)),Stats.Speed/1600,Stats.MaxStamina/160};
        for(int32 J=0;J<5;++J){const float SY=Y+53*S+J*21*S;Text(Labels[J],X+CardW*.57f,SY,.49f,Muted);Bar(X+CardW*.78f,SY+3*S,CardW*.18f,5*S,FMath::Clamp(Ratings[J],0.f,1.f),Accent);}
        Text(Ability[I],X+12*S,Y+CardH-23*S,.57f,Muted);
    }
    Text(TEXT("Choose a card or press 1-6 to start a new round. Raptor followers stay allied; only the leader awards a kill."),Left,H*.754f,.64f,Muted);
    Text(GM&&GM->bPerformanceMap?TEXT("MAP: SUNGRASS PLAINS - PERFORMANCE  /  CLICK TO CHANGE"):TEXT("MAP: SUNGRASS PLAINS - STANDARD  /  CLICK TO CHANGE"),Left,H*.775f,.88f,Gold);
    DrawRect(FLinearColor(.11f,.23f,.22f),W*.18f,H*.83f,W*.40f,H*.08f);
    Text(TEXT("ENTER / ESC   RESUME EXPLORATION"),W*.22f,H*.852f,.97f,Teal);
    DrawRect(FLinearColor(.18f,.09f,.065f),W*.62f,H*.83f,W*.20f,H*.08f);
    Text(TEXT("F10   QUIT"),W*.67f,H*.852f,.97f,Gold);
    Text(TEXT("WASD move   |   Space jump   |   Q / E pivot  |  Ctrl brace   |   LMB quick   |   RMB charge   |   F eat"),Left,H*.945f,.76f,Muted);
}
void ADinoHUD::DrawWorldMap(ADinosaurCharacter* D,bool Full)
{
    float S=Scale,W=Canvas->SizeX,H=Canvas->SizeY;
    float Size=Full?FMath::Min(H*.70f,W*.64f):166*S;
    float X=Full?(W-Size)*.5f:W-Size-30*S,Y=Full?(H-Size)*.5f:25*S;
    Panel(X-10*S,Y-10*S,Size+20*S,Size+(Full?78:20)*S,.94f);
    if(WorldMap)DrawTextureSimple(WorldMap,X,Y,Size/512.f);else DrawRect(FLinearColor(.14f,.21f,.15f),X,Y,Size,Size);
    auto Point=[&](FVector P){return FVector2D(X+(P.X/60000.f+.5f)*Size,Y+(.5f-P.Y/60000.f)*Size);};
    if(Full)
    {
        for(FVector P:ALostValleyWorld::Landmarks()){auto Q=Point(P);DrawRect(Gold,Q.X-2*S,Q.Y-2*S,4*S,4*S);Text(ALostValleyWorld::RegionName(P),Q.X+6*S,Q.Y,.65f,Gold);}
        const auto* GM=GetWorld()->GetGameState<ADinoGameState>();
        Text(GM&&GM->bTeamMatch?TEXT("Allies: always / Enemies: sight or noise   N ^"):TEXT("Markers: sight / recent attack or sprint   N ^"),X,Y+Size+18*S,.70f,Muted);
        Text(TEXT("M Close   Point + R: add/remove pin   Max 8"),X,Y+Size+41*S,.70f,Gold);
    }
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(*It!=D&&It->bMajor&&!It->bDead)
    {
        FVector Marker;if(!D->MapPositionFor(*It,Marker))continue;
        auto Q=Point(Marker);DrawRect(D->IsEnemy(*It)?Red:Teal,Q.X-2*S,Q.Y-2*S,4*S,4*S);
    }
    if(const auto* PC=Cast<ADinoPlayerController>(PlayerOwner))for(const FVector& Pin:PC->MapPins)
    {
        auto P=Point(Pin);const float R=Full?7*S:5*S;
        DrawLine(P.X,P.Y-R,P.X+R,P.Y,Gold,2*S);DrawLine(P.X+R,P.Y,P.X,P.Y+R,Gold,2*S);
        DrawLine(P.X,P.Y+R,P.X-R,P.Y,Gold,2*S);DrawLine(P.X-R,P.Y,P.X,P.Y-R,Gold,2*S);
    }
    const auto Q=Point(D->GetActorLocation());
    const FVector Direction=D->GetVelocity().SizeSquared2D()>FMath::Square(35.f)?D->GetVelocity().GetSafeNormal2D():D->GetActorForwardVector();
    const FVector2D F(Direction.X,-Direction.Y),R(-F.Y,F.X),Tip=Q+F*20*S;
    // Black position disc plus a separate directional arrow, with a thin light rim.
    auto Arrow=[&](FLinearColor Color,float Width){
        DrawLine(Q.X,Q.Y,Tip.X,Tip.Y,Color,Width*S);
        const auto Left=Tip-F*7*S+R*5*S,Right=Tip-F*7*S-R*5*S;
        DrawLine(Tip.X,Tip.Y,Left.X,Left.Y,Color,Width*S);DrawLine(Tip.X,Tip.Y,Right.X,Right.Y,Color,Width*S);
    };
    Arrow(FLinearColor::White,5);Arrow(FLinearColor::Black,2.5f);
    for(int32 Layer=0;Layer<2;++Layer){const float Radius=(Layer==0?7.f:5.5f)*S;for(float Row=-Radius;Row<=Radius;Row+=1){const float HalfWidth=FMath::Sqrt(FMath::Max(0.f,Radius*Radius-Row*Row));DrawLine(Q.X-HalfWidth,Q.Y+Row,Q.X+HalfWidth,Q.Y+Row,Layer==0?FLinearColor::White:FLinearColor::Black,1.5f);}}
}
