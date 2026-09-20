#include "DinoHUD.h"
#include "DinosaurCharacter.h"
#include "DinoPlayerController.h"
#include "DinoGameMode.h"
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
    Super::DrawHUD();if(!Canvas)return;auto* D=Cast<ADinosaurCharacter>(GetOwningPawn());auto* PC=Cast<ADinoPlayerController>(PlayerOwner);if(!D||!PC)return;
    Scale=FMath::Clamp(Canvas->SizeY/900.f,.45f,1.5f);float S=Scale,W=Canvas->SizeX,H=Canvas->SizeY;
    if(!bMapLoaded){WorldMap=LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/T_ValleyMap.T_ValleyMap"));bMapLoaded=true;}
    if(PC->bSelectionOpen){DrawMenu(D,PC);return;}
    if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())
    {
        auto Score=GM->GetScore(D->CombatantID);
        Panel(24*S,H-188*S,246*S,75*S);
        Text(TEXT("KILLS     DEATHS     ASSISTS"),40*S,H-176*S,.69f,Muted);
        Text(FString::Printf(TEXT("%d          %d          %d"),Score.Kills,Score.Deaths,Score.Assists),43*S,H-151*S,1.35f,Gold);
        FString Goal=GM->bTeamMatch?FString::Printf(TEXT("5v5   YOUR TEAM %d - %d RIVALS   /   GOAL %d"),GM->TeamKills[0],GM->TeamKills[1],GM->TeamKillGoal):FString::Printf(TEXT("FREE-FOR-ALL   %d / %d KILLS"),Score.Kills,GM->SoloKillGoal);
        Text(Goal,W*.5f-160*S,79*S,.73f,Teal);
    }
    Panel(24*S,24*S,386*S,230*S);
    Text(TEXT("DINOSAUR BATTLE  /  0.1"),42*S,37*S,.95f,Gold);
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
    Text(ALostValleyWorld::RegionName(D->GetActorLocation()),W*.5f-150*S,30*S,.98f,Gold);
    Text(D->bSwimming?TEXT("SWIMMING - SPACE TO SURGE"):D->bInWater?TEXT("WADING - MOVEMENT SLOWED"):TEXT("LOST VALLEY"),W*.5f-80*S,51*S,.70f,D->bInWater?Teal:Muted);
    if(D->Species==1)
    {
        int32 Alive=0,Close=0;
        for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(*It!=D&&It->Species==1&&!It->bDead&&!D->IsEnemy(*It)){++Alive;if(FVector::Dist2D(D->GetActorLocation(),It->GetActorLocation())<5000)++Close;}
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
        FVector2D P;if(PlayerOwner->ProjectWorldLocationToScreen(O->GetActorLocation()+FVector(0,0,O->Stats().HalfHeight+80),P)&&P.X>0&&P.X<W&&P.Y>0&&P.Y<H)
        {
            FLinearColor C=D->IsEnemy(O)?Red:Teal;Bar(P.X-45*S,P.Y,90*S,5*S,O->Health->Fraction(),C);
            FString Name=O->Stats().Name;
            if(O->Species==1)if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())Name=GM->IsScoringTarget(O)?TEXT("RAPTOR LEADER"):TEXT("Pack follower");
            Text(Name,P.X-45*S,P.Y-19*S,.58f,C);
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
        Text(D->Food->bEating?TEXT("FEEDING  +HEALTH / STAMINA / HUNGER"):D->Species==2?TEXT("HOLD E - EAT VEGETATION"):TEXT("HOLD E - FEED ON CARCASS"),W*.5f-185*S,H-164*S,.76f,Teal);
        AActor* Meal=D->Food->FindFood(D->Stats().AttackRange+260);float Remaining=0;
        if(auto* Plant=Cast<AFoodPlant>(Meal))Remaining=Plant->Nutrition;else if(auto* Corpse=Cast<ADinosaurCarcass>(Meal))Remaining=Corpse->Nutrition;
        Text(FString::Printf(TEXT("%.0f food remaining"),Remaining),W*.5f-185*S,H-138*S,.7f,Muted);
    }
    if(PC->bShowHelp)
    {
        Panel(24*S,H-99*S,W-48*S,75*S,.82f);
        Text(TEXT("WASD Move   SHIFT Sprint   MOUSE Look   SPACE Jump   HOLD Q Brace"),42*S,H-84*S,.94f);
        Text(TEXT("LMB  Quick attack     HOLD / RELEASE RMB  Heavy attack     HOLD E  Eat"),42*S,H-60*S,.88f,Muted);
        Text(TEXT("1 / 2 / 3  Species     M  Map     H  Help     ESC  Pause"),42*S,H-38*S,.78f,Gold);
    }
    else Text(TEXT("H  Help     M  Map     ESC  Pause"),30*S,H-32*S,.8f,Muted);
    DrawWorldMap(D,PC->bMapOpen);
}
void ADinoHUD::DrawMenu(ADinosaurCharacter* D,ADinoPlayerController* PC)
{
    float W=Canvas->SizeX,H=Canvas->SizeY,S=Scale;Panel(0,0,W,H,.93f);
    Text(TEXT("DINOSAUR BATTLE"),W*.105f,H*.11f,2.7f,Gold);
    Text(TEXT("LOST VALLEY  /  SINGLE PLAYER  /  PRE-ALPHA 0.1"),W*.108f,H*.19f,.75f,Muted);
    auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>();
    if(GM&&GM->bRoundOver&&!PC->bSettingsOpen)
    {
        Text(GM->WinnerName(),W*.20f,H*.27f,1.7f,Gold);
        Text(GM->bTeamMatch?TEXT("TEAM RESULTS"):FString::Printf(TEXT("SOLO RESULTS / FIRST TO %d KILLS"),GM->SoloKillGoal),W*.20f,H*.235f,.78f,Teal);
        Text(TEXT("PLAYER / DINOSAUR"),W*.20f,H*.34f,.78f,Muted);
        Text(TEXT("KILLS"),W*.59f,H*.34f,.72f,Muted);Text(TEXT("DEATHS"),W*.67f,H*.34f,.72f,Muted);Text(TEXT("ASSISTS"),W*.76f,H*.34f,.72f,Muted);
        TArray<int32> IDs;GM->Scores.GetKeys(IDs);IDs.Sort([&](int32 A,int32 B){auto SA=GM->GetScore(A),SB=GM->GetScore(B);return SA.Kills!=SB.Kills?SA.Kills>SB.Kills:SA.Deaths!=SB.Deaths?SA.Deaths<SB.Deaths:A<B;});
        float Y=H*.39f;
        for(int32 ID:IDs)
        {
            auto* Who=GM->FindCombatant(ID);if(!Who)continue;auto Score=GM->GetScore(ID);FLinearColor C=ID==0?Teal:Muted;
            FString Name=(ID==0?TEXT("YOU / "):FString::Printf(TEXT("AI %d / "),ID))+Who->Stats().Name;
            if(Who->Species==1&&!GM->IsScoringTarget(Who))Name+=TEXT(" follower");
            Text(Name,W*.20f,Y,.76f,C);Text(FString::FromInt(Score.Kills),W*.59f,Y,.84f,C);Text(FString::FromInt(Score.Deaths),W*.67f,Y,.84f,C);Text(FString::FromInt(Score.Assists),W*.76f,Y,.84f,C);Y+=27*S;
        }
        Text(TEXT("ENTER / ESC  Play again     1 / 2 / 3  Change dinosaur     F3  Change mode"),W*.20f,H*.85f,.78f,Gold);
        return;
    }
    if(GM)Text(TEXT("F3  ")+GM->MatchName(),W*.108f,H*.245f,.88f,Teal);
    Text(PC->bSettingsOpen?TEXT("F2  BACK"):TEXT("F2  SETTINGS"),W*.76f,H*.13f,.95f,Teal);
    if(PC->bSettingsOpen)
    {
        Panel(W*.18f,H*.28f,W*.64f,H*.43f,1);
        Text(TEXT("SETTINGS"),W*.22f,H*.30f,1.4f,Gold);
        Text(TEXT("BLOOD EFFECTS"),W*.22f,H*.37f,1.15f);
        Text(PC->bBloodEnabled?TEXT("[ B ]   ON"):TEXT("[ B ]   OFF"),W*.64f,H*.37f,1.15f,PC->bBloodEnabled?Red:Teal);
        Text(TEXT("Optional blood particles when a dinosaur or prey is hit."),W*.22f,H*.425f,.84f,Muted);
        Text(FString::Printf(TEXT("MOUSE SENSITIVITY     %.1f"),D->MouseSensitivity),W*.22f,H*.525f,1.08f);
        Text(TEXT("[-]       [+]"),W*.65f,H*.525f,1.15f,Gold);
        Text(TEXT("Settings save automatically on this computer."),W*.22f,H*.64f,.8f,Muted);
        Text(TEXT("ESC / F2  Back to dinosaur selection      ENTER  Resume"),W*.22f,H*.78f,.95f,Teal);
        return;
    }
    float Top=H*.28f,CardW=W*.25f,CardH=400*S,Gap=W*.035f,Left=(W-3*CardW-2*Gap)*.5f;
    if(Portraits.Num()!=3){Portraits.SetNumZeroed(3);const TCHAR* Kinds[]={TEXT("Trex"),TEXT("Raptor"),TEXT("Trike")};for(int32 I=0;I<3;++I)Portraits[I]=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/UI/T_%sPortrait.T_%sPortrait"),Kinds[I],Kinds[I]));}
    const TCHAR* Names[]={TEXT("TYRANNOSAURUS"),TEXT("VELOCIRAPTOR"),TEXT("TRICERATOPS")};
    const TCHAR* Roles[]={TEXT("SOLO PREDATOR"),TEXT("PACK LEADER"),TEXT("DEFENSIVE HERBIVORE")};
    const TCHAR* Abilities[]={TEXT("Crushing bite / charged lunge"),TEXT("Fast slash / leaping strike"),TEXT("Horn thrust / powerful brace")};
    const TCHAR* Foods[]={TEXT("Feed on fallen prey"),TEXT("Lead an allied raptor pack"),TEXT("Feed on cycad clusters")};
    const float Ratings[3][3]={{.9f,.55f,.60f},{.70f,.95f,.30f},{.75f,.45f,.90f}};
    float MX=-1,MY=-1;PC->GetMousePosition(MX,MY);
    for(int32 I=0;I<3;++I)
    {
        float X=Left+I*(CardW+Gap);bool Hover=MX>X&&MX<X+CardW&&MY>Top&&MY<Top+CardH;
        FLinearColor Accent=I==1?Teal:I==2?FLinearColor(.8f,.48f,.28f):Gold;
        DrawRect(Hover?FLinearColor(.10f,.17f,.17f,1):FLinearColor(.055f,.085f,.09f,1),X,Top,CardW,CardH);
        DrawRect(Accent,X,Top,CardW,4*S);Text(FString::Printf(TEXT("%d"),I+1),X+CardW-35*S,Top+14*S,1.0f,Accent);
        Text(Names[I],X+20*S,Top+14*S,1.02f);Text(Roles[I],X+20*S,Top+41*S,.68f,Accent);
        if(Portraits.IsValidIndex(I)&&Portraits[I])DrawTexture(Portraits[I],X+10*S,Top+65*S,CardW-20*S,(CardW-20*S)*.5625f,0,0,1,1,FLinearColor::White,BLEND_Translucent);
        const TCHAR* Stats[]={TEXT("ATTACK"),TEXT("SPEED"),TEXT("TOUGHNESS")};
        for(int32 J=0;J<3;++J){float Y=Top+(285+25*J)*S;Text(Stats[J],X+20*S,Y,.67f,Muted);Bar(X+110*S,Y+3*S,CardW-132*S,7*S,Ratings[I][J],Accent);}
        Text(Abilities[I],X+20*S,Top+362*S,.72f);Text(Foods[I],X+20*S,Top+383*S,.68f,Muted);
    }
    Text(TEXT("Choose a card or press 1, 2, 3 to start a new round. Only raptor leaders award kills."),Left,H*.75f,.70f,Muted);
    Text(FString::Printf(TEXT("Mouse sensitivity  %.1f    [- / +] adjust"),D->MouseSensitivity),Left,H*.775f,.88f,Gold);
    DrawRect(FLinearColor(.11f,.23f,.22f),W*.18f,H*.83f,W*.40f,H*.08f);
    Text(TEXT("ENTER / ESC   RESUME EXPLORATION"),W*.22f,H*.852f,.97f,Teal);
    DrawRect(FLinearColor(.18f,.09f,.065f),W*.62f,H*.83f,W*.20f,H*.08f);
    Text(TEXT("F10   QUIT"),W*.67f,H*.852f,.97f,Gold);
    Text(TEXT("WASD move   |   Space jump   |   Q brace   |   LMB quick   |   RMB charge   |   E eat"),Left,H*.945f,.76f,Muted);
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
        const auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>();
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
    auto Q=Point(D->GetActorLocation());float A=FMath::DegreesToRadians(D->GetActorRotation().Yaw);FVector2D F(FMath::Cos(A),-FMath::Sin(A)),R(-F.Y,F.X);
    FVector2D Tip=Q+F*9*S,L=Q-F*5*S+R*5*S,B=Q-F*5*S-R*5*S;
    DrawLine(Tip.X,Tip.Y,L.X,L.Y,FLinearColor::White,2*S);DrawLine(L.X,L.Y,B.X,B.Y,FLinearColor::White,2*S);DrawLine(B.X,B.Y,Tip.X,Tip.Y,FLinearColor::White,2*S);
}
