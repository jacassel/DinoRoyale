#include "DinoPlayerController.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "CombatComponent.h"
#include "DinoAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/Material.h"
#include "DinosaurAIController.h"
#include "LostValleyWorld.h"
#include "FoodSystem.h"
#include "DinoEffects.h"
#include "DinoGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Json.h"
#include "HighResScreenshot.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "TimerManager.h"
#include "Misc/ConfigCacheIni.h"
#include "EngineUtils.h"

void ADinoPlayerController::BeginPlay()
{
    Super::BeginPlay();
    bDevBridge=FParse::Param(FCommandLine::Get(),TEXT("DinoDevBridge"));
    BridgeRoot=FPaths::ProjectSavedDir()/TEXT("Automation");
    if(bDevBridge)
    {
        IFileManager::Get().MakeDirectory(*BridgeRoot,true);
        FString Existing;TSharedPtr<FJsonObject> Old;
        if(FFileHelper::LoadFileToString(Existing,*(BridgeRoot/TEXT("command.json")))&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Existing),Old)&&Old.IsValid())LastSequence=Old->GetIntegerField(TEXT("seq"));
    }
    GConfig->GetBool(TEXT("Dino.UserSettings"),TEXT("BloodEnabled"),bBloodEnabled,GGameIni);
    ConsoleCommand(TEXT("t.MaxFPS 60"),false);
    SetInputMode(FInputModeGameOnly()); bShowMouseCursor=false;
    PrimaryActorTick.bTickEvenWhenPaused=true;bShouldPerformFullTickWhenPaused=true;
    if(!bDevBridge||FParse::Param(FCommandLine::Get(),TEXT("DinoStartMenu")))GetWorldTimerManager().SetTimerForNextTick([this](){SetMenuOpen(true);});
}
void ADinoPlayerController::PlayerTick(float Dt)
{
    Super::PlayerTick(Dt);
    if(!bDevBridge)return;
    if(Dt>0){FrameSum+=Dt;FrameCount++;}
    const double Now=FPlatformTime::Seconds();
    if(Now-LastBridgeTime>=.05){LastBridgeTime=Now;ReadBridge();WriteTelemetry();}
}
void ADinoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindAction("Menu",IE_Pressed,this,&ADinoPlayerController::ToggleMenu).bExecuteWhenPaused=true;
    InputComponent->BindAction("SelectRex",IE_Pressed,this,&ADinoPlayerController::SelectRex).bExecuteWhenPaused=true;
    InputComponent->BindAction("SelectRaptor",IE_Pressed,this,&ADinoPlayerController::SelectRaptor).bExecuteWhenPaused=true;
    InputComponent->BindAction("SelectTrike",IE_Pressed,this,&ADinoPlayerController::SelectTrike).bExecuteWhenPaused=true;
    auto& Click=InputComponent->BindAction("QuickAttack",IE_Pressed,this,&ADinoPlayerController::MenuClick);Click.bExecuteWhenPaused=true;Click.bConsumeInput=false;
    InputComponent->BindAction("Confirm",IE_Pressed,this,&ADinoPlayerController::ResumeGame).bExecuteWhenPaused=true;
    InputComponent->BindAction("Quit",IE_Pressed,this,&ADinoPlayerController::QuitGame).bExecuteWhenPaused=true;
    InputComponent->BindAction("SensitivityUp",IE_Pressed,this,&ADinoPlayerController::SensitivityUp).bExecuteWhenPaused=true;
    InputComponent->BindAction("SensitivityDown",IE_Pressed,this,&ADinoPlayerController::SensitivityDown).bExecuteWhenPaused=true;
    InputComponent->BindAction("MatchMode",IE_Pressed,this,&ADinoPlayerController::ToggleMatchMode).bExecuteWhenPaused=true;
    InputComponent->BindAction("Settings",IE_Pressed,this,&ADinoPlayerController::ToggleSettings).bExecuteWhenPaused=true;
    InputComponent->BindAction("Blood",IE_Pressed,this,&ADinoPlayerController::ToggleBlood).bExecuteWhenPaused=true;
    InputComponent->BindAction("Map",IE_Pressed,this,&ADinoPlayerController::ToggleMap);
    InputComponent->BindAction("Help",IE_Pressed,this,&ADinoPlayerController::ToggleHelp);
    InputComponent->BindAction("Respawn",IE_Pressed,this,&ADinoPlayerController::RespawnPlayer);
}
void ADinoPlayerController::SetMenuOpen(bool Open)
{
    bSelectionOpen=Open;bMapOpen=false;if(!Open)bSettingsOpen=false;
    if(Open)
    {
        if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->Combat->bCharging=false;D->Combat->bBracing=false;D->Combat->BufferedQuick=0;D->SprintOff();D->Food->StopEating();D->GetCharacterMovement()->StopMovementImmediately();}
        FlushPressedKeys();SetPause(true);SetInputMode(FInputModeGameAndUI().SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock).SetHideCursorDuringCapture(false));bShowMouseCursor=true;
    }
    else{SetPause(false);SetInputMode(FInputModeGameOnly());bShowMouseCursor=false;FlushPressedKeys();}
}
void ADinoPlayerController::ToggleMenu(){if(bSettingsOpen){bSettingsOpen=false;return;}if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GM->bRoundOver){ResumeGame();return;}SetMenuOpen(!bSelectionOpen);}
void ADinoPlayerController::ToggleMatchMode(){if(bSelectionOpen&&!bSettingsOpen)if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->SetTeamMode(!GM->bTeamMatch);}
void ADinoPlayerController::ToggleSettings(){if(bSelectionOpen)bSettingsOpen=!bSettingsOpen;}
void ADinoPlayerController::ToggleBlood(){if(bSelectionOpen&&bSettingsOpen){bBloodEnabled=!bBloodEnabled;if(!bBloodEnabled)ADinoEffects::ClearBlood(GetWorld());GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("BloodEnabled"),bBloodEnabled,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::ToggleMap(){if(!bSelectionOpen)bMapOpen=!bMapOpen;}
void ADinoPlayerController::ToggleHelp(){bShowHelp=!bShowHelp;}
void ADinoPlayerController::SelectRex(){DinoSpecies(0);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->StartRound();SetMenuOpen(false);}
void ADinoPlayerController::SelectRaptor(){DinoSpecies(1);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->StartRound();SetMenuOpen(false);}
void ADinoPlayerController::SelectTrike(){DinoSpecies(2);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->StartRound();SetMenuOpen(false);}
void ADinoPlayerController::ResumeGame(){if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GM->bRoundOver)GM->StartRound();if(bSelectionOpen)SetMenuOpen(false);}
void ADinoPlayerController::QuitGame(){if(bSelectionOpen)ConsoleCommand(TEXT("quit"));}
void ADinoPlayerController::SensitivityUp(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->MouseSensitivity=FMath::Clamp(D->MouseSensitivity+.1f,.2f,3.f);GConfig->SetFloat(TEXT("Dino.Session"),TEXT("MouseSensitivity"),D->MouseSensitivity,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::SensitivityDown(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->MouseSensitivity=FMath::Clamp(D->MouseSensitivity-.1f,.2f,3.f);GConfig->SetFloat(TEXT("Dino.Session"),TEXT("MouseSensitivity"),D->MouseSensitivity,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::RespawnPlayer(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))if(!D->bDead)D->Die();}
void ADinoPlayerController::MenuClick()
{
    if(!bSelectionOpen)return;float X,Y;if(!GetMousePosition(X,Y))return;int32 W,H;GetViewportSize(W,H);
    if(X>W*.73f&&Y>H*.1f&&Y<H*.21f){ToggleSettings();return;}
    if(bSettingsOpen){if(Y>H*.33f&&Y<H*.45f)ToggleBlood();else if(Y>H*.50f&&Y<H*.59f){if(X>W*.6f)SensitivityUp();else SensitivityDown();}return;}
    float S=FMath::Clamp(H/900.f,.45f,1.5f),Top=H*.28f,CardW=W*.25f,CardH=400*S,Gap=W*.035f,Left=(W-3*CardW-2*Gap)*.5f;
    if(Y>H*.215f&&Y<H*.285f){ToggleMatchMode();return;}
    if(Y>=Top&&Y<=Top+CardH)for(int32 I=0;I<3;++I)if(X>=Left+I*(CardW+Gap)&&X<=Left+I*(CardW+Gap)+CardW){DinoSpecies(I);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->StartRound();SetMenuOpen(false);return;}
    if(Y>H*.83f&&Y<H*.91f){if(X>W*.62f)QuitGame();else if(X>W*.18f)ResumeGame();}
}
void ADinoPlayerController::DinoSpecies(int32 I){if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->ApplySpecies(I);D->ResetLife();}}
void ADinoPlayerController::DinoDamage(float A){if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))D->ReceiveHit(A,nullptr);}
void ADinoPlayerController::DinoHeal(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))D->Health->Heal(D->Health->Maximum);}
void ADinoPlayerController::DinoTeleport(float X,float Y)
{
    if(auto* D=Cast<ADinosaurCharacter>(GetPawn()))
    {
        FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(D);
        float Z=500;
        if(GetWorld()->LineTraceSingleByChannel(Hit,FVector(X,Y,30000),FVector(X,Y,-30000),ECC_WorldStatic,Q))Z=Hit.ImpactPoint.Z+D->Stats().HalfHeight+10;
        D->SetActorLocation(FVector(X,Y,Z),false,nullptr,ETeleportType::TeleportPhysics);
        D->GetCharacterMovement()->StopMovementImmediately();
    }
}
void ADinoPlayerController::DinoSnapshot(){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/Windows/DinosaurBattle.png"),true,true);}
void ADinoPlayerController::ReadBridge()
{
    FString Text;if(!FFileHelper::LoadFileToString(Text,*(BridgeRoot/TEXT("command.json"))))return;
    TSharedPtr<FJsonObject> O;auto Reader=TJsonReaderFactory<>::Create(Text);
    if(!FJsonSerializer::Deserialize(Reader,O)||!O.IsValid())return;
    int32 Seq=O->GetIntegerField(TEXT("seq"));if(Seq<=LastSequence)return;LastSequence=Seq;
    FString Cmd=O->GetStringField(TEXT("cmd"));auto* D=Cast<ADinosaurCharacter>(GetPawn());if(!D)return;
    if(Cmd==TEXT("key"))
    {
        FKey Key(*O->GetStringField(TEXT("key"))); FString Event=O->GetStringField(TEXT("event"));
        float Value=1;double Number;if(O->TryGetNumberField(TEXT("value"),Number))Value=Number;
        FInputKeyEventArgs Args(nullptr,FInputDeviceId::CreateFromInternalId(0),Key,Event==TEXT("down")?IE_Pressed:Event==TEXT("up")?IE_Released:IE_Axis,Value,false,FPlatformTime::Cycles64());
        Args.DeltaTime=.05f;Args.NumSamples=1;InputKey(Args);
    }
    else if(Cmd==TEXT("species")){DinoSpecies(O->GetIntegerField(TEXT("value")));DinoTeleport(0,0);}
    else if(Cmd==TEXT("damage"))DinoDamage(O->GetNumberField(TEXT("value")));
    else if(Cmd==TEXT("stamina")){D->Stamina->Current=FMath::Clamp(float(O->GetNumberField(TEXT("value"))),0.f,D->Stamina->Maximum);D->Stamina->bExhausted=D->Stamina->Current<=0;D->Stamina->ExhaustionLeft=D->Stamina->bExhausted?D->Stats().ExhaustionDuration:0;}
    else if(Cmd==TEXT("heal"))DinoHeal();
    else if(Cmd==TEXT("respawn"))D->ResetLife();
    else if(Cmd==TEXT("teleport"))DinoTeleport(O->GetNumberField(TEXT("x")),O->GetNumberField(TEXT("y")));
    else if(Cmd==TEXT("face")){float Yaw=O->GetNumberField(TEXT("yaw"));SetControlRotation(FRotator(-13,Yaw,0));D->SetActorRotation(FRotator(0,Yaw,0));D->GetCharacterMovement()->StopMovementImmediately();}
    else if(Cmd==TEXT("target"))
    {
        if(!IsValid(TestTarget))
        {
            FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            TestTarget=GetWorld()->SpawnActor<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),D->GetActorLocation()+D->GetActorForwardVector()*D->Stats().AttackRange,FRotator::ZeroRotator,P);
        }
        if(TestTarget)
        {
            TestTarget->ApplySpecies(D->Species==0?2:0);TestTarget->ResetLife();TestTarget->bMajor=false;
            TestTarget->Health->Reset(10000,1000,0);double TestTeam;TestTarget->TeamID=O->TryGetNumberField(TEXT("team"),TestTeam)?int32(TestTeam):-1;
            TestTarget->SetActorLocation(D->GetActorLocation()+D->GetActorForwardVector()*(D->Stats().AttackRange*.75f));
            TestTarget->SetActorRotation((D->GetActorLocation()-TestTarget->GetActorLocation()).Rotation());
        }
    }
    else if(Cmd==TEXT("ai"))
    {
        bool Paused=O->GetBoolField(TEXT("paused"));
        for(TActorIterator<ADinosaurAIController> It(GetWorld());It;++It)
        {
            It->bPaused=Paused;
            if(auto* Other=Cast<ADinosaurCharacter>(It->GetPawn())){Other->Combat->Cancel();Other->Food->StopEating();Other->GetCharacterMovement()->StopMovementImmediately();}
        }
    }
    else if(Cmd==TEXT("invulnerable"))D->Health->bInvulnerable=O->GetBoolField(TEXT("value"));
    else if(Cmd==TEXT("removeTarget")){if(IsValid(TestTarget)){TestTarget->Destroy();TestTarget=nullptr;}}
    else if(Cmd==TEXT("hitFromTarget"))
    {
        if(IsValid(TestTarget))
        {
            bool Front=O->GetBoolField(TEXT("front"));
            TestTarget->SetActorLocation(D->GetActorLocation()+D->GetActorForwardVector()*(Front?1:-1)*D->Stats().AttackRange);
            D->ReceiveHit(O->GetNumberField(TEXT("value")),TestTarget);
        }
    }
    else if(Cmd==TEXT("food"))
    {
        FVector P=D->GetActorLocation()+D->GetActorForwardVector()*(D->Stats().AttackRange*.65f);
        P.Z=ALostValleyWorld::HeightAt(P.X,P.Y);
        if(D->Species==2)GetWorld()->SpawnActor<AFoodPlant>(P,FRotator::ZeroRotator);
        else
        {
            FActorSpawnParameters S;S.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            auto* Prey=GetWorld()->SpawnActor<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),P+FVector(0,0,FSpeciesData::Get(3).HalfHeight),FRotator::ZeroRotator,S);
            Prey->ApplySpecies(3);Prey->bMajor=false;Prey->RespawnDelay=0;Prey->ReceiveHit(10000,D);
        }
    }
    else if(Cmd==TEXT("route"))
    {
        for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It)
        {
            FVector End=It->NearestWalkable(FVector(O->GetNumberField(TEXT("x")),O->GetNumberField(TEXT("y")),0));
            TArray<FVector> Path;bool Success=It->FindPath(D->GetActorLocation(),End,Path);
            auto R=MakeShared<FJsonObject>();R->SetBoolField(TEXT("success"),Success);TArray<TSharedPtr<FJsonValue>> Points;
            for(auto P:Path){auto Pt=MakeShared<FJsonObject>();Pt->SetNumberField(TEXT("x"),P.X);Pt->SetNumberField(TEXT("y"),P.Y);Pt->SetNumberField(TEXT("z"),P.Z);Points.Add(MakeShared<FJsonValueObject>(Pt));}
            R->SetArrayField(TEXT("points"),Points);FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(R,W);FFileHelper::SaveStringToFile(Out,*(BridgeRoot/TEXT("route.json")));break;
        }
    }
    else if(Cmd==TEXT("navAudit"))
    {
        for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It)
        {
            TArray<TSharedPtr<FJsonValue>> Routes;auto Marks=ALostValleyWorld::Landmarks();
            for(int32 A=0;A<Marks.Num();++A)for(int32 B=0;B<Marks.Num();++B)if(A!=B)
            {
                TArray<FVector> Path;bool Success=It->FindPath(It->NearestWalkable(Marks[A]),It->NearestWalkable(Marks[B]),Path);
                auto Row=MakeShared<FJsonObject>();Row->SetNumberField(TEXT("from"),A);Row->SetNumberField(TEXT("to"),B);Row->SetBoolField(TEXT("success"),Success);Row->SetNumberField(TEXT("waypoints"),Path.Num());
                Routes.Add(MakeShared<FJsonValueObject>(Row));
            }
            auto R=MakeShared<FJsonObject>();R->SetArrayField(TEXT("routes"),Routes);FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(R,W);FFileHelper::SaveStringToFile(Out,*(BridgeRoot/TEXT("navigation-audit.json")));break;
        }
    }
    else if(Cmd==TEXT("match")){if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->SetTeamMode(O->GetBoolField(TEXT("teams")));SetMenuOpen(false);}
    else if(Cmd==TEXT("sandbox")){if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->bIgnoreWinCondition=O->GetBoolField(TEXT("enabled"));}
    else if(Cmd==TEXT("scoreHit"))
    {
        auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>();int32 A=O->GetIntegerField(TEXT("attacker")),V=O->GetIntegerField(TEXT("victim"));
        ADinosaurCharacter* Victim=nullptr;for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->CombatantID==V){Victim=*It;break;}
        if(GM&&Victim)Victim->ReceiveHit(O->GetNumberField(TEXT("value")),GM->FindCombatant(A));
    }
    else if(Cmd==TEXT("resetCombatant")){if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(auto* Other=GM->FindCombatant(O->GetIntegerField(TEXT("id"))))Other->ResetLife();}
    else if(Cmd==TEXT("travelAI"))
    {
        if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(auto* Other=GM->FindCombatant(O->GetIntegerField(TEXT("id"))))
        {
            double SX,SY;if(O->TryGetNumberField(TEXT("startX"),SX)&&O->TryGetNumberField(TEXT("startY"),SY))
            {
                Other->ResetLife();Other->SetActorLocation(FVector(SX,SY,ALostValleyWorld::HeightAt(SX,SY)+Other->Stats().HalfHeight+20));Other->GetCharacterMovement()->StopMovementImmediately();
            }
            if(auto* AI=Cast<ADinosaurAIController>(Other->GetController())){AI->bPaused=false;AI->SetTravelGoal(FVector(O->GetNumberField(TEXT("x")),O->GetNumberField(TEXT("y")),0));}
        }
    }
    else if(Cmd==TEXT("testAI"))
    {
        if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(auto* Other=GM->FindCombatant(O->GetIntegerField(TEXT("id"))))
        {
            Other->ApplySpecies(O->GetIntegerField(TEXT("species")));Other->ResetLife();
            float X=O->GetNumberField(TEXT("x")),Y=O->GetNumberField(TEXT("y"));Other->SetActorLocation(FVector(X,Y,ALostValleyWorld::HeightAt(X,Y)+Other->Stats().HalfHeight+12));Other->SetActorRotation(FRotator(0,O->GetNumberField(TEXT("yaw")),0));Other->GetCharacterMovement()->StopMovementImmediately();
            Other->Health->Current=Other->Health->Maximum*O->GetNumberField(TEXT("health"));Other->Health->LastDamageTime=GetWorld()->GetTimeSeconds();
            if(auto* AI=Cast<ADinosaurAIController>(Other->GetController())){AI->ClearTravelGoal();AI->ResetTactics();double Profile;if(O->TryGetNumberField(TEXT("personality"),Profile))AI->Personality=FMath::Clamp(int32(Profile),0,3);AI->bPaused=!O->GetBoolField(TEXT("enabled"));}
        }
    }
    else if(Cmd==TEXT("enableAI")){if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(auto* Other=GM->FindCombatant(O->GetIntegerField(TEXT("id"))))if(auto* AI=Cast<ADinosaurAIController>(Other->GetController()))AI->bPaused=!O->GetBoolField(TEXT("enabled"));}
    else if(Cmd==TEXT("camera")){FRotator R=GetControlRotation();R.Yaw=O->GetNumberField(TEXT("yaw"));SetControlRotation(R);}
    else if(Cmd==TEXT("menu"))SetMenuOpen(O->GetBoolField(TEXT("open")));
    else if(Cmd==TEXT("screenshot"))DinoSnapshot();
    else if(Cmd==TEXT("quit"))ConsoleCommand(TEXT("quit"));
}
void ADinoPlayerController::WriteTelemetry()
{
    auto* D=Cast<ADinosaurCharacter>(GetPawn());if(!D)return;
    auto O=MakeShared<FJsonObject>();O->SetNumberField(TEXT("seq"),LastSequence);O->SetNumberField(TEXT("time"),GetWorld()->GetTimeSeconds());
    O->SetNumberField(TEXT("species"),D->Species);O->SetNumberField(TEXT("health"),D->Health->Current);O->SetNumberField(TEXT("maxHealth"),D->Health->Maximum);
    FVector L=D->GetActorLocation();O->SetNumberField(TEXT("x"),L.X);O->SetNumberField(TEXT("y"),L.Y);O->SetNumberField(TEXT("z"),L.Z);
    O->SetNumberField(TEXT("speed"),D->GetVelocity().Size2D());O->SetNumberField(TEXT("maxSpeed"),D->GetCharacterMovement()->MaxWalkSpeed);
    O->SetBoolField(TEXT("keyW"),IsInputKeyDown(EKeys::W));O->SetBoolField(TEXT("ignoreMove"),IsMoveInputIgnored());
    O->SetBoolField(TEXT("swimming"),D->bSwimming);O->SetNumberField(TEXT("waterSurface"),D->WaterSurface);O->SetNumberField(TEXT("swimSpeed"),D->GetCharacterMovement()->MaxSwimSpeed);
    O->SetNumberField(TEXT("acceleration"),D->GetCharacterMovement()->GetCurrentAcceleration().Size2D());
    O->SetNumberField(TEXT("floorNormalZ"),D->GetCharacterMovement()->CurrentFloor.HitResult.ImpactNormal.Z);
    O->SetStringField(TEXT("floorActor"),GetNameSafe(D->GetCharacterMovement()->CurrentFloor.HitResult.GetActor()));
    O->SetNumberField(TEXT("yaw"),D->GetActorRotation().Yaw);O->SetNumberField(TEXT("cameraYaw"),GetControlRotation().Yaw);O->SetNumberField(TEXT("cameraPitch"),GetControlRotation().Pitch);
    O->SetBoolField(TEXT("brace"),D->Combat->bBracing);O->SetBoolField(TEXT("charging"),D->Combat->bCharging);O->SetBoolField(TEXT("dead"),D->bDead);
    O->SetBoolField(TEXT("falling"),D->GetCharacterMovement()->IsFalling());O->SetNumberField(TEXT("charge"),D->Combat->ChargeFraction());
    O->SetNumberField(TEXT("recovery"),D->Combat->RecoveryLeft);O->SetNumberField(TEXT("attackDuration"),D->Combat->AttackDuration);O->SetNumberField(TEXT("hits"),D->Combat->TotalHits);
    O->SetNumberField(TEXT("stamina"),D->Stamina->Current);O->SetNumberField(TEXT("maxStamina"),D->Stamina->Maximum);O->SetBoolField(TEXT("exhausted"),D->Stamina->bExhausted);O->SetBoolField(TEXT("sprinting"),D->bSprinting);
    O->SetNumberField(TEXT("combo"),D->Combat->ComboCount);O->SetNumberField(TEXT("attackSerial"),D->Combat->AttackSerial);O->SetNumberField(TEXT("attackElapsed"),D->Combat->AttackElapsed);O->SetBoolField(TEXT("weakAttack"),D->Combat->bWeakAttack);
    O->SetNumberField(TEXT("damageDealt"),D->Combat->LastDealtDamage);
    O->SetNumberField(TEXT("targetHealth"),IsValid(TestTarget)?TestTarget->Health->Current:-1);
    O->SetNumberField(TEXT("meanFPS"),FrameSum>0?FrameCount/FrameSum:0);
    O->SetNumberField(TEXT("frameCount"),FrameCount);O->SetNumberField(TEXT("frameSeconds"),FrameSum);
    int32 Major=0;for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor)Major++;
    O->SetNumberField(TEXT("majorCount"),Major);O->SetNumberField(TEXT("respawnDelay"),D->RespawnDelay);
    if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())
    {
        auto Score=GM->GetScore(D->CombatantID);O->SetNumberField(TEXT("kills"),Score.Kills);O->SetNumberField(TEXT("deaths"),Score.Deaths);O->SetNumberField(TEXT("assists"),Score.Assists);
        O->SetBoolField(TEXT("teamMode"),GM->bTeamMatch);O->SetBoolField(TEXT("roundOver"),GM->bRoundOver);O->SetNumberField(TEXT("winner"),GM->WinnerID);O->SetNumberField(TEXT("winnerTeam"),GM->WinnerTeam);
        O->SetNumberField(TEXT("team0Kills"),GM->TeamKills[0]);O->SetNumberField(TEXT("team1Kills"),GM->TeamKills[1]);O->SetNumberField(TEXT("goal"),GM->bTeamMatch?GM->TeamKillGoal:GM->SoloKillGoal);
        TArray<TSharedPtr<FJsonValue>> Board;for(auto Pair:GM->Scores){auto Row=MakeShared<FJsonObject>();Row->SetNumberField(TEXT("id"),Pair.Key);Row->SetNumberField(TEXT("kills"),Pair.Value.Kills);Row->SetNumberField(TEXT("deaths"),Pair.Value.Deaths);Row->SetNumberField(TEXT("assists"),Pair.Value.Assists);Board.Add(MakeShared<FJsonValueObject>(Row));}O->SetArrayField(TEXT("scoreboard"),Board);
    }
    for(TActorIterator<ADinoEffects> It(GetWorld());It;++It){O->SetNumberField(TEXT("bloodParticles"),It->ActiveCount());O->SetNumberField(TEXT("bloodEmitted"),It->TotalEmitted);}
    O->SetBoolField(TEXT("settingsOpen"),bSettingsOpen);O->SetBoolField(TEXT("bloodEnabled"),bBloodEnabled);O->SetBoolField(TEXT("inWater"),D->bInWater);O->SetBoolField(TEXT("menuOpen"),bSelectionOpen);O->SetBoolField(TEXT("mapOpen"),bMapOpen);O->SetNumberField(TEXT("sensitivity"),D->MouseSensitivity);O->SetStringField(TEXT("animation"),D->Animation->State);O->SetBoolField(TEXT("eating"),D->Food->bEating);O->SetNumberField(TEXT("foodConsumed"),D->Food->FoodConsumed);
    TArray<TSharedPtr<FJsonValue>> Materials;
    for(int32 I=0;I<D->GetMesh()->GetNumMaterials();++I)if(auto* M=D->GetMesh()->GetMaterial(I))Materials.Add(MakeShared<FJsonValueString>(GetNameSafe(M->GetMaterial())));
    O->SetArrayField(TEXT("materials"),Materials);
    O->SetStringField(TEXT("region"),ALostValleyWorld::RegionName(L));
    TArray<TSharedPtr<FJsonValue>> AIs;
    for(TActorIterator<ADinosaurAIController> It(GetWorld());It;++It)
    {
        auto* Other=Cast<ADinosaurCharacter>(It->GetPawn());if(!Other)continue;
        auto Row=MakeShared<FJsonObject>();Row->SetNumberField(TEXT("team"),Other->TeamID);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())Row->SetBoolField(TEXT("scoringTarget"),GM->IsScoringTarget(Other));Row->SetNumberField(TEXT("id"),Other->CombatantID);Row->SetNumberField(TEXT("species"),Other->Species);Row->SetBoolField(TEXT("major"),Other->bMajor);
        Row->SetNumberField(TEXT("personality"),It->Personality);Row->SetNumberField(TEXT("stamina"),Other->Stamina->Current);Row->SetBoolField(TEXT("exhausted"),Other->Stamina->bExhausted);Row->SetBoolField(TEXT("sprinting"),Other->bSprinting);Row->SetNumberField(TEXT("combo"),Other->Combat->ComboCount);Row->SetNumberField(TEXT("attackSerial"),Other->Combat->AttackSerial);Row->SetNumberField(TEXT("recovery"),Other->Combat->RecoveryLeft);Row->SetBoolField(TEXT("charging"),Other->Combat->bCharging);
        Row->SetStringField(TEXT("decision"),It->Decision);Row->SetNumberField(TEXT("fightConfidence"),It->FightConfidence);Row->SetNumberField(TEXT("escapeConfidence"),It->EscapeConfidence);Row->SetNumberField(TEXT("guards"),It->GuardsUsed);Row->SetNumberField(TEXT("retreats"),It->RetreatDecisions);Row->SetNumberField(TEXT("retaliations"),It->Retaliations);
        FVector P=Other->GetActorLocation();Row->SetNumberField(TEXT("x"),P.X);Row->SetNumberField(TEXT("y"),P.Y);Row->SetNumberField(TEXT("z"),P.Z);Row->SetNumberField(TEXT("ground"),ALostValleyWorld::HeightAt(P.X,P.Y));
        Row->SetStringField(TEXT("state"),It->State);Row->SetStringField(TEXT("animation"),Other->Animation->State);Row->SetBoolField(TEXT("dead"),Other->bDead);Row->SetBoolField(TEXT("swimming"),Other->bSwimming);
        Row->SetNumberField(TEXT("health"),Other->Health->Current);Row->SetNumberField(TEXT("maxHealth"),Other->Health->Maximum);
        Row->SetNumberField(TEXT("speed"),Other->GetVelocity().Size2D());Row->SetNumberField(TEXT("yaw"),Other->GetActorRotation().Yaw);
        Row->SetNumberField(TEXT("hits"),Other->Combat->TotalHits);Row->SetNumberField(TEXT("attacks"),It->AttacksMade);Row->SetNumberField(TEXT("stuckRecoveries"),It->StuckRecoveries);
        Row->SetNumberField(TEXT("failedPaths"),It->FailedPaths);Row->SetNumberField(TEXT("distance"),It->DistanceTravelled);
        Row->SetNumberField(TEXT("target"),It->Target.IsValid()?It->Target->CombatantID:-1);Row->SetNumberField(TEXT("leader"),It->Leader.IsValid()?It->Leader->CombatantID:-1);
        AIs.Add(MakeShared<FJsonValueObject>(Row));
    }
    O->SetArrayField(TEXT("ai"),AIs);
    FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(O,W);
    FFileHelper::SaveStringToFile(Out,*(BridgeRoot/TEXT("telemetry.json")));
}
