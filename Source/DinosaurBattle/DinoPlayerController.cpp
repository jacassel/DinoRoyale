#include "DinoPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "DinoOnlineSession.h"
#include "DinoPlayerState.h"
#include "DinoAudioComponent.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "HungerComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
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
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
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
    if(!IsLocalController())return;
    ALostValleyWorld::EnsureLocalScene(GetWorld());
    if(GetNetMode()!=NM_Standalone)
    {OnlinePage=4;if(auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>()){Online->EnterNetworkWorld();ServerDisplayName(Online->Nickname());}}
    bDevBridge=FParse::Param(FCommandLine::Get(),TEXT("DinoDevBridge"));
    BridgeRoot=FPaths::ProjectSavedDir()/TEXT("Automation");
    FString BridgeName;if(FParse::Value(FCommandLine::Get(),TEXT("DinoBridge="),BridgeName))BridgeRoot=FPaths::ProjectSavedDir()/TEXT("Automation")/FPaths::MakeValidFileName(BridgeName);
    if(bDevBridge)
    {
        IFileManager::Get().MakeDirectory(*BridgeRoot,true);
        FString Existing;TSharedPtr<FJsonObject> Old;
        if(FFileHelper::LoadFileToString(Existing,*(BridgeRoot/TEXT("command.json")))&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Existing),Old)&&Old.IsValid())LastSequence=Old->GetIntegerField(TEXT("seq"));
    }
    GConfig->GetBool(TEXT("Dino.UserSettings"),TEXT("BloodEnabled"),bBloodEnabled,GGameIni);
    GConfig->GetBool(TEXT("Dino.UserSettings"),TEXT("ShowNameTags"),bShowNameTags,GGameIni);
    ConsoleCommand(TEXT("t.MaxFPS 60"),false);
    SetInputMode(FInputModeGameOnly()); bShowMouseCursor=false;
    PrimaryActorTick.bTickEvenWhenPaused=true;bShouldPerformFullTickWhenPaused=true;
    if(!bDevBridge||FParse::Param(FCommandLine::Get(),TEXT("DinoStartMenu")))GetWorldTimerManager().SetTimerForNextTick([this](){SetMenuOpen(true);});
}
void ADinoPlayerController::PlayerTick(float Dt)
{
    Super::PlayerTick(Dt);
    if(!IsLocalController())return;
    if(GetNetMode()==NM_Standalone)if(auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>())if(Online->bShowMenuOnReturn)
    {Online->bShowMenuOnReturn=false;OnlinePage=1;SetMenuOpen(true);}
    if(bWaitingForRoundStart)if(auto* GS=GetWorld()->GetGameState<ADinoGameState>())if(!GS->bLobby&&!GS->bRoundOver){bWaitingForRoundStart=false;SetMenuOpen(false);}
    if(!bDevBridge)return;
    if(Dt>0){FrameSum+=Dt;FrameCount++;}
    const double Now=FPlatformTime::Seconds();
    if(Now-LastBridgeTime>=.05){LastBridgeTime=Now;ReadBridge();WriteTelemetry();}
}
void ADinoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::N,IE_Pressed,this,&ADinoPlayerController::ToggleNameTags).bExecuteWhenPaused=true;
    InputComponent->BindKey(EKeys::F4,IE_Pressed,this,&ADinoPlayerController::ToggleMultiplayer).bExecuteWhenPaused=true;
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
    InputComponent->BindAction("MapPin",IE_Pressed,this,&ADinoPlayerController::PlaceMapPin);
}
void ADinoPlayerController::SetMenuOpen(bool Open)
{
    if(!IsLocalController())return;
    bSelectionOpen=Open;bMapOpen=false;if(!Open)bSettingsOpen=false;
    ResetIgnoreLookInput();ResetIgnoreMoveInput();
    if(Open)
    {
        if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->CancelActions();D->GetCharacterMovement()->StopMovementImmediately();}
        FlushPressedKeys();if(GetNetMode()==NM_Standalone)SetPause(true);SetInputMode(FInputModeGameAndUI().SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock).SetHideCursorDuringCapture(false));bShowMouseCursor=true;
    }
    else{if(GetNetMode()==NM_Standalone)SetPause(false);SetInputMode(FInputModeGameOnly());bShowMouseCursor=false;FlushPressedKeys();}
}
void ADinoPlayerController::ToggleMenu(){if(OnlinePage>0&&GetNetMode()==NM_Standalone){OnlinePage=OnlinePage==1?0:1;return;}if(GetNetMode()!=NM_Standalone){if(auto* GS=GetWorld()->GetGameState<ADinoGameState>())if(GS->bLobby||GS->bRoundOver)return;SetMenuOpen(!bSelectionOpen);return;}if(bSettingsOpen){bSettingsOpen=false;return;}if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GM->bRoundOver){ResumeGame();return;}SetMenuOpen(!bSelectionOpen);}
void ADinoPlayerController::ToggleMatchMode(){if(bSelectionOpen&&!bSettingsOpen)if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())GM->SetTeamMode(!GM->bTeamMatch);}
void ADinoPlayerController::ToggleSettings(){if(bSelectionOpen)bSettingsOpen=!bSettingsOpen;}
void ADinoPlayerController::ToggleNameTags(){if(bSelectionOpen&&bSettingsOpen){bShowNameTags=!bShowNameTags;GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("ShowNameTags"),bShowNameTags,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::ToggleBlood(){if(bSelectionOpen&&bSettingsOpen){bBloodEnabled=!bBloodEnabled;if(!bBloodEnabled)ADinoEffects::ClearBlood(GetWorld());GConfig->SetBool(TEXT("Dino.UserSettings"),TEXT("BloodEnabled"),bBloodEnabled,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::ToggleMap()
{
    if(bSelectionOpen)return;
    bMapOpen=!bMapOpen;FlushPressedKeys();
    SetIgnoreLookInput(bMapOpen);SetIgnoreMoveInput(bMapOpen);bShowMouseCursor=bMapOpen;
    if(bMapOpen)
    {
        if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->CancelActions();D->GetCharacterMovement()->StopMovementImmediately();}
        SetInputMode(FInputModeGameAndUI().SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock).SetHideCursorDuringCapture(false));
        int32 W,H;GetViewportSize(W,H);SetMouseLocation(W/2,H/2);
    }
    else SetInputMode(FInputModeGameOnly());
}
void ADinoPlayerController::PlaceMapPin()
{
    if(bSelectionOpen||!bMapOpen)return;
    float MX,MY;if(!GetMousePosition(MX,MY))return;
    int32 W,H;GetViewportSize(W,H);const float Size=FMath::Min(H*.70f,W*.64f),X=(W-Size)*.5f,Y=(H-Size)*.5f;
    if(MX<X||MX>X+Size||MY<Y||MY>Y+Size)return;
    const FVector P(((MX-X)/Size-.5f)*60000.f,(.5f-(MY-Y)/Size)*60000.f,0);
    const float Radius=12.f*FMath::Clamp(H/900.f,.45f,1.5f)*60000.f/Size;
    for(int32 I=0;I<MapPins.Num();++I)if(FVector::Dist2D(P,MapPins[I])<Radius){MapPins.RemoveAt(I);return;}
    if(MapPins.Num()>=8)MapPins.RemoveAt(0);
    MapPins.Add(P);
}
void ADinoPlayerController::ToggleHelp(){bShowHelp=!bShowHelp;}
void ADinoPlayerController::SelectRex(){ChooseSpecies(0);}
void ADinoPlayerController::SelectRaptor(){ChooseSpecies(1);}
void ADinoPlayerController::SelectTrike(){ChooseSpecies(2);}
void ADinoPlayerController::ResumeGame(){if(OnlinePage>0&&GetNetMode()==NM_Standalone)return;if(GetNetMode()!=NM_Standalone){if(auto* GS=GetWorld()->GetGameState<ADinoGameState>())if(GS->bLobby||GS->bRoundOver)return;SetMenuOpen(false);return;}if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GM->bRoundOver)GM->StartRound();if(bSelectionOpen)SetMenuOpen(false);}
void ADinoPlayerController::QuitGame(){if(bSelectionOpen)ConsoleCommand(TEXT("quit"));}
void ADinoPlayerController::SensitivityUp(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->MouseSensitivity=FMath::Clamp(D->MouseSensitivity+.1f,.2f,3.f);GConfig->SetFloat(TEXT("Dino.Session"),TEXT("MouseSensitivity"),D->MouseSensitivity,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::SensitivityDown(){if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->MouseSensitivity=FMath::Clamp(D->MouseSensitivity-.1f,.2f,3.f);GConfig->SetFloat(TEXT("Dino.Session"),TEXT("MouseSensitivity"),D->MouseSensitivity,GGameIni);GConfig->Flush(false,GGameIni);}}
void ADinoPlayerController::MenuClick()
{
    if(!bSelectionOpen)return;float X,Y;if(!GetMousePosition(X,Y))return;int32 W,H;GetViewportSize(W,H);
    if(!bSettingsOpen&&(OnlinePage>0||GetNetMode()!=NM_Standalone)){OnlineClick(X/W,Y/H);return;}
    if(X>W*.72f&&Y>H*.215f&&Y<H*.28f){ToggleMultiplayer();return;}
    if(const auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GM->bRoundOver&&!bSettingsOpen)return;
    if(X>W*.73f&&Y>H*.1f&&Y<H*.21f){ToggleSettings();return;}
    if(bSettingsOpen){if(Y>H*.60f&&Y<H*.67f){ToggleNameTags();return;}if(Y>H*.33f&&Y<H*.45f)ToggleBlood();else if(Y>H*.50f&&Y<H*.59f){if(X>W*.6f)SensitivityUp();else SensitivityDown();}return;}
    float S=FMath::Clamp(H/900.f,.45f,1.5f),Top=H*.28f,CardW=W*.25f,CardH=400*S,Gap=W*.035f,Left=(W-3*CardW-2*Gap)*.5f;
    if(Y>H*.215f&&Y<H*.285f){ToggleMatchMode();return;}
    if(Y>=Top&&Y<=Top+CardH)for(int32 I=0;I<3;++I)if(X>=Left+I*(CardW+Gap)&&X<=Left+I*(CardW+Gap)+CardW){DinoSpecies(I);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GetNetMode()==NM_Standalone)GM->StartRound();SetMenuOpen(false);return;}
    if(Y>H*.83f&&Y<H*.91f){if(X>W*.62f)QuitGame();else if(X>W*.18f)ResumeGame();}
}
void ADinoPlayerController::DinoSpecies(int32 I){if(GetNetMode()!=NM_Standalone)return;if(auto* D=Cast<ADinosaurCharacter>(GetPawn())){D->ApplySpecies(I);D->ResetLife();}}
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
    else if(Cmd==TEXT("mouse"))SetMouseLocation(O->GetIntegerField(TEXT("x")),O->GetIntegerField(TEXT("y")));
    else if(Cmd==TEXT("audioRecord"))
    {
        if(O->GetBoolField(TEXT("start")))UAudioMixerBlueprintLibrary::StartRecordingOutput(this,60);
        else UAudioMixerBlueprintLibrary::StopRecordingOutput(this,EAudioRecordingExportType::WavFile,FString::Printf(TEXT("Species%d"),D->Species),FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AudioQA")));
    }
    else if(Cmd==TEXT("species")){DinoSpecies(O->GetIntegerField(TEXT("value")));DinoTeleport(0,0);}
    else if(Cmd==TEXT("damage"))DinoDamage(O->GetNumberField(TEXT("value")));
    else if(Cmd==TEXT("hunger"))
    {
        ADinosaurCharacter* Subject=D;double ID;if(O->TryGetNumberField(TEXT("id"),ID))if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())Subject=GM->FindCombatant(int32(ID));
        if(Subject)Subject->Hunger->Current=FMath::Clamp(float(O->GetNumberField(TEXT("value"))),0.f,Subject->Hunger->Maximum);
    }
    else if(Cmd==TEXT("aiAbility"))
    {
        if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(auto* A=GM->FindCombatant(O->GetIntegerField(TEXT("id"))))
        {FString Action=O->GetStringField(TEXT("action"));if(Action==TEXT("quick"))A->Combat->QuickAttack();else if(Action==TEXT("charge"))A->Combat->StartCharge();else if(Action==TEXT("release"))A->Combat->ReleaseCharge();}
    }
    else if(Cmd==TEXT("sightBlocker"))
    {
        if(IsValid(TestSightBlocker)){TestSightBlocker->Destroy();TestSightBlocker=nullptr;}
        if(O->GetBoolField(TEXT("enabled")))
        {
            auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>();TestSightBlocker=Wall;
            Wall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
            Wall->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
            Wall->SetActorLocation(D->GetActorLocation()+FVector(800,0,200));Wall->SetActorScale3D(FVector(1,20,20));
            Wall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Wall->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);Wall->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
        }
    }
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
    else if(Cmd==TEXT("clearTestFood"))
    {
        for(TActorIterator<ADinosaurCarcass> It(GetWorld());It;++It)It->Destroy();
        for(TActorIterator<AFoodPlant> It(GetWorld());It;++It)if(It->Tags.Contains(TEXT("TestFood")))It->Destroy();
    }
    else if(Cmd==TEXT("food"))
    {
        FVector P=D->GetActorLocation()+D->GetActorForwardVector()*(D->Stats().AttackRange*.65f);
        P.Z=ALostValleyWorld::HeightAt(P.X,P.Y);
        if(D->Species==2){if(auto* Plant=GetWorld()->SpawnActor<AFoodPlant>(P,FRotator::ZeroRotator))Plant->Tags.Add(TEXT("TestFood"));}
        else
        {
            FActorSpawnParameters S;S.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            auto* Prey=GetWorld()->SpawnActor<ADinosaurCharacter>(ADinosaurCharacter::StaticClass(),P+FVector(0,0,FSpeciesData::Get(3).HalfHeight),FRotator::ZeroRotator,S);
            double FoodSpecies;Prey->ApplySpecies(O->TryGetNumberField(TEXT("species"),FoodSpecies)?int32(FoodSpecies):3);Prey->CombatantID=-99;Prey->bMajor=false;Prey->SetActorLocation(P+FVector(0,0,Prey->Stats().HalfHeight+10));Prey->RespawnDelay=0;Prey->ReceiveHit(10000,D);
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
    else if(Cmd==TEXT("worldAudit"))
    {
        for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It)
        {
            auto R=MakeShared<FJsonObject>();R->SetNumberField(TEXT("trees"),It->Trunks->GetInstanceCount());R->SetNumberField(TEXT("obstacles"),It->Obstacles.Num());
            TArray<TSharedPtr<FJsonValue>> Samples;
            for(float Y=-24000;Y<=24000;Y+=1000)for(float X=-24000;X<=24000;X+=1000)
            {
                auto P=MakeShared<FJsonObject>();P->SetNumberField(TEXT("x"),X);P->SetNumberField(TEXT("y"),Y);P->SetNumberField(TEXT("z"),ALostValleyWorld::HeightAt(X,Y));
                const float DX=ALostValleyWorld::HeightAt(X+150,Y)-ALostValleyWorld::HeightAt(X-150,Y),DY=ALostValleyWorld::HeightAt(X,Y+150)-ALostValleyWorld::HeightAt(X,Y-150);
                P->SetNumberField(TEXT("slope"),FMath::Sqrt(DX*DX+DY*DY)/300);Samples.Add(MakeShared<FJsonValueObject>(P));
            }
            R->SetArrayField(TEXT("terrain"),Samples);FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(R,W);FFileHelper::SaveStringToFile(Out,*(BridgeRoot/TEXT("world-audit.json")));break;
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
            double HungerValue,StaminaValue;if(O->TryGetNumberField(TEXT("hunger"),HungerValue))Other->Hunger->Current=HungerValue;if(O->TryGetNumberField(TEXT("stamina"),StaminaValue))Other->Stamina->Current=StaminaValue;
            if(auto* AI=Cast<ADinosaurAIController>(Other->GetController())){AI->ClearTravelGoal();AI->ResetTactics();double Profile;if(O->TryGetNumberField(TEXT("personality"),Profile))AI->Personality=FMath::Clamp(int32(Profile),0,3);AI->bPaused=!O->GetBoolField(TEXT("enabled"));}
        }
    }
    else if(Cmd==TEXT("duelSetup"))
    {
        // Isolate real AI combat; no changes to stamina, hunger, damage, injury or ability rules.
        auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>();if(!GM)return;
        GM->bTeamMatch=false;GM->bIgnoreWinCondition=true;GM->bRoundOver=false;
        D->ApplySpecies(2);DinoTeleport(-25000,-25000);D->bDead=true;D->RespawnDelay=0;D->Health->Current=0;
        const int32 Seed=O->GetIntegerField(TEXT("seed"));FRandomStream R(Seed);
        const float Angle=O->GetNumberField(TEXT("angle"));const int32 Opponent=O->GetIntegerField(TEXT("opponent"));
        const bool Pack=O->GetBoolField(TEXT("pack"));
        for(TActorIterator<ADinosaurAIController> It(GetWorld());It;++It)
        {
            auto* A=Cast<ADinosaurCharacter>(It->GetPawn());if(!A)continue;
            const int32 ID=A->CombatantID;const bool Active=ID>=1&&ID<=(Pack?4:2);
            A->ApplySpecies(ID==1?Opponent:Active?1:3);A->ResetLife();A->RespawnDelay=0;
            It->ResetTactics();It->ClearTravelGoal();It->SetTestSeed(Seed+ID*173);It->Personality=(Seed+ID)%4;It->bPaused=!Active;
            FVector P=Active?FVector(ID==1?-650:650,ID<=2?0:ID==3?-500:500,0).RotateAngleAxis(Angle,FVector::UpVector):FVector(-25000+ID*30,-24000,0);
            P.Z=ALostValleyWorld::HeightAt(P.X,P.Y)+A->Stats().HalfHeight+15;A->SetActorLocation(P);A->HomePosition=P;
            A->SetActorRotation(FRotator(0,Angle+(ID==1?R.FRandRange(-60,60):180+R.FRandRange(-45,45)),0));
            A->GetCharacterMovement()->StopMovementImmediately();
            A->GetCapsuleComponent()->SetCollisionEnabled(Active?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
            if(!Active){A->bDead=true;A->Health->Current=0;A->SetActorHiddenInGame(true);}
            else A->SetActorHiddenInGame(false);
        }
        for(TActorIterator<ADinosaurCarcass> It(GetWorld());It;++It)It->Destroy();
    }
    else if(Cmd==TEXT("enableAI")){if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(auto* Other=GM->FindCombatant(O->GetIntegerField(TEXT("id"))))if(auto* AI=Cast<ADinosaurAIController>(Other->GetController()))AI->bPaused=!O->GetBoolField(TEXT("enabled"));}
    else if(Cmd==TEXT("camera"))
    {
        FRotator R=GetControlRotation();R.Yaw=O->GetNumberField(TEXT("yaw"));
        if(O->HasField(TEXT("pitch")))R.Pitch=O->GetNumberField(TEXT("pitch"));
        if(O->HasField(TEXT("distance")))D->CameraBoom->TargetArmLength=O->GetNumberField(TEXT("distance"));
        SetControlRotation(R);
    }
    else if(Cmd==TEXT("lobby"))ServerLobbyAction(uint8(O->GetIntegerField(TEXT("action"))),O->GetIntegerField(TEXT("value")));
    else if(Cmd==TEXT("online"))
    {
        // This path only runs behind the existing opt-in DinoDevBridge.
        if(auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>())
        {
            const FString Action=O->GetStringField(TEXT("action"));
            if(Action==TEXT("signin"))Online->SignIn();
            else if(Action==TEXT("host"))Online->Host(O->GetBoolField(TEXT("teams")),O->GetIntegerField(TEXT("capacity")),O->GetBoolField(TEXT("bots")),O->GetBoolField(TEXT("public")));
            else if(Action==TEXT("search"))Online->Search();
            else if(Action==TEXT("join"))Online->Join(O->GetIntegerField(TEXT("index")));
            else if(Action==TEXT("invite"))Online->InviteFriends();
            else if(Action==TEXT("leave"))Online->Leave();
        }
    }
    else if(Cmd==TEXT("menu"))SetMenuOpen(O->GetBoolField(TEXT("open")));
    else if(Cmd==TEXT("screenshot"))DinoSnapshot();
    else if(Cmd==TEXT("quit"))ConsoleCommand(TEXT("quit"));
}
void ADinoPlayerController::WriteTelemetry()
{
    auto* D=Cast<ADinosaurCharacter>(GetPawn());if(!D)return;
    auto O=MakeShared<FJsonObject>();O->SetNumberField(TEXT("seq"),LastSequence);O->SetNumberField(TEXT("time"),GetWorld()->GetTimeSeconds());
    O->SetNumberField(TEXT("netMode"),GetNetMode());O->SetNumberField(TEXT("combatantID"),D->CombatantID);
    TArray<TSharedPtr<FJsonValue>> NetworkActors;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto A=MakeShared<FJsonObject>();A->SetNumberField(TEXT("id"),It->CombatantID);A->SetNumberField(TEXT("species"),It->Species);
        A->SetNumberField(TEXT("health"),It->Health->Current);A->SetNumberField(TEXT("stamina"),It->Stamina->Current);A->SetBoolField(TEXT("dead"),It->bDead);
        A->SetNumberField(TEXT("x"),It->GetActorLocation().X);A->SetNumberField(TEXT("y"),It->GetActorLocation().Y);A->SetNumberField(TEXT("z"),It->GetActorLocation().Z);
        A->SetNumberField(TEXT("yaw"),It->GetActorRotation().Yaw);A->SetNumberField(TEXT("pivot"),It->PivotVisual);
        A->SetNumberField(TEXT("halfHeight"),It->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
        A->SetNumberField(TEXT("meshRelativeZ"),It->GetMesh()->GetRelativeLocation().Z);
        A->SetNumberField(TEXT("meshBaseZ"),It->GetBaseTranslationOffset().Z);
        A->SetNumberField(TEXT("meshWorldZ"),It->GetMesh()->GetComponentLocation().Z);
        A->SetNumberField(TEXT("ground"),ALostValleyWorld::HeightAt(It->GetActorLocation().X,It->GetActorLocation().Y));
        A->SetNumberField(TEXT("speed"),It->GetVelocity().Size2D());
        A->SetBoolField(TEXT("swimming"),It->bSwimming);
        A->SetBoolField(TEXT("brace"),It->Combat->bBracing);
        A->SetStringField(TEXT("animation"),It->Animation->State);
        FVector MP=FVector::ZeroVector;A->SetBoolField(TEXT("mapVisible"),D->MapPositionFor(*It,MP));A->SetNumberField(TEXT("markerX"),MP.X);A->SetNumberField(TEXT("markerY"),MP.Y);A->SetBoolField(TEXT("follower"),It->bPackFollower);A->SetNumberField(TEXT("pack"),It->PackLeaderID);A->SetBoolField(TEXT("scoring"),It->bScoringParticipant);A->SetBoolField(TEXT("enemy"),D->IsEnemy(*It));A->SetBoolField(TEXT("bot"),It->bFillerBot);A->SetNumberField(TEXT("team"),It->TeamID);A->SetNumberField(TEXT("attackSerial"),It->Combat->AttackSerial);A->SetBoolField(TEXT("player"),It->IsPlayerControlled());NetworkActors.Add(MakeShared<FJsonValueObject>(A));
    }
    O->SetArrayField(TEXT("networkActors"),NetworkActors);
    if(auto* Online=GetGameInstance()->GetSubsystem<UDinoOnlineSession>())
    {
        O->SetStringField(TEXT("onlineStatus"),Online->Status);
        O->SetBoolField(TEXT("onlineSignedIn"),Online->IsSignedIn());O->SetBoolField(TEXT("onlineBusy"),Online->bBusy);
        O->SetNumberField(TEXT("compatibilityBuild"),Online->BuildVersion);O->SetNumberField(TEXT("onlineResults"),Online->Results.Num());
    }
    O->SetNumberField(TEXT("onlinePage"),OnlinePage);
    O->SetNumberField(TEXT("frameMs"),GetWorld()->GetDeltaSeconds()*1000.0);
    int32 ActorCount=0;for(TActorIterator<AActor> It(GetWorld());It;++It)++ActorCount;
    O->SetNumberField(TEXT("actorCount"),ActorCount);
    for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It)
    {
        O->SetNumberField(TEXT("treeInstances"),It->Trunks->GetInstanceCount());
        O->SetNumberField(TEXT("grassInstances"),It->Grass->GetInstanceCount());
        O->SetNumberField(TEXT("fernInstances"),It->Ferns->GetInstanceCount());
        O->SetNumberField(TEXT("navObstacles"),It->Obstacles.Num());
        break;
    }
    if(auto* Driver=GetWorld()->GetNetDriver())
    {
        Driver->bCollectNetStats=true;
        O->SetNumberField(TEXT("netInBytes"),Driver->InTotalBytes);O->SetNumberField(TEXT("netOutBytes"),Driver->OutTotalBytes);
        O->SetNumberField(TEXT("netInBytesPerSecond"),Driver->InBytesPerSecond);O->SetNumberField(TEXT("netOutBytesPerSecond"),Driver->OutBytesPerSecond);
        O->SetNumberField(TEXT("netInPacketsLost"),Driver->InTotalPacketsLost);O->SetNumberField(TEXT("netOutPacketsLost"),Driver->OutTotalPacketsLost);
#if DO_ENABLE_NET_TEST
        O->SetNumberField(TEXT("emulatedLagMs"),Driver->PacketSimulationSettings.PktLag);
        O->SetNumberField(TEXT("emulatedLossPercent"),Driver->PacketSimulationSettings.PktLoss);
#endif
    }
    if(auto* PS=GetPlayerState<ADinoPlayerState>())O->SetNumberField(TEXT("pingMs"),PS->GetPingInMilliseconds());
    TArray<TSharedPtr<FJsonValue>> Pins;for(const FVector& P:MapPins){auto Pin=MakeShared<FJsonObject>();Pin->SetNumberField(TEXT("x"),P.X);Pin->SetNumberField(TEXT("y"),P.Y);Pins.Add(MakeShared<FJsonValueObject>(Pin));}O->SetArrayField(TEXT("mapPins"),Pins);
    O->SetNumberField(TEXT("audioSteps"),D->Audio->Steps);O->SetNumberField(TEXT("audioQuick"),D->Audio->QuickSounds);O->SetNumberField(TEXT("audioHeavy"),D->Audio->HeavySounds);O->SetNumberField(TEXT("audioImpacts"),D->Audio->Impacts);O->SetNumberField(TEXT("audioDeaths"),D->Audio->Deaths);O->SetNumberField(TEXT("audioVoices"),D->Audio->ActiveVoices());
    O->SetNumberField(TEXT("audioCharges"),D->Audio->Charges);O->SetNumberField(TEXT("audioHurts"),D->Audio->Hurts);
    O->SetNumberField(TEXT("audioSprintBreaths"),D->Audio->SprintBreaths);O->SetNumberField(TEXT("audioInjuredBreaths"),D->Audio->InjuredBreaths);O->SetNumberField(TEXT("audioLoadedClips"),D->Audio->LoadedClips);
    int32 AudioTotal=0;for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->Audio)AudioTotal+=It->Audio->ActiveVoices();O->SetNumberField(TEXT("audioTotalVoices"),AudioTotal);
    O->SetNumberField(TEXT("species"),D->Species);O->SetNumberField(TEXT("health"),D->Health->Current);O->SetNumberField(TEXT("maxHealth"),D->Health->Maximum);
    FVector L=D->GetActorLocation();O->SetNumberField(TEXT("x"),L.X);O->SetNumberField(TEXT("y"),L.Y);O->SetNumberField(TEXT("z"),L.Z);
    O->SetNumberField(TEXT("pivot"),D->PivotVisual);O->SetNumberField(TEXT("pivotInput"),D->PivotInput);O->SetNumberField(TEXT("speed"),D->GetVelocity().Size2D());O->SetNumberField(TEXT("maxSpeed"),D->GetCharacterMovement()->MaxWalkSpeed);
    O->SetBoolField(TEXT("keyW"),IsInputKeyDown(EKeys::W));O->SetBoolField(TEXT("ignoreMove"),IsMoveInputIgnored());
    O->SetBoolField(TEXT("swimming"),D->bSwimming);O->SetNumberField(TEXT("waterSurface"),D->WaterSurface);O->SetNumberField(TEXT("swimSpeed"),D->GetCharacterMovement()->MaxSwimSpeed);
    O->SetNumberField(TEXT("acceleration"),D->GetCharacterMovement()->GetCurrentAcceleration().Size2D());
    O->SetNumberField(TEXT("floorNormalZ"),D->GetCharacterMovement()->CurrentFloor.HitResult.ImpactNormal.Z);
    O->SetStringField(TEXT("floorActor"),GetNameSafe(D->GetCharacterMovement()->CurrentFloor.HitResult.GetActor()));
    O->SetNumberField(TEXT("yaw"),D->GetActorRotation().Yaw);O->SetNumberField(TEXT("cameraYaw"),GetControlRotation().Yaw);O->SetNumberField(TEXT("cameraPitch"),GetControlRotation().Pitch);
    O->SetBoolField(TEXT("brace"),D->Combat->bBracing);O->SetBoolField(TEXT("charging"),D->Combat->bCharging);O->SetBoolField(TEXT("dead"),D->bDead);
    O->SetBoolField(TEXT("falling"),D->GetCharacterMovement()->IsFalling());O->SetNumberField(TEXT("charge"),D->Combat->ChargeFraction());
    O->SetNumberField(TEXT("recovery"),D->Combat->RecoveryLeft);O->SetNumberField(TEXT("attackDuration"),D->Combat->AttackDuration);O->SetNumberField(TEXT("hits"),D->Combat->TotalHits);
    O->SetNumberField(TEXT("hunger"),D->Hunger->Current);O->SetNumberField(TEXT("healthRegenFactor"),D->Hunger->HealthRegenFactor());O->SetNumberField(TEXT("staminaRegenFactor"),D->Hunger->StaminaRegenFactor());O->SetNumberField(TEXT("revealUntil"),D->RevealUntil);
    O->SetNumberField(TEXT("stamina"),D->Stamina->Current);O->SetNumberField(TEXT("maxStamina"),D->Stamina->Maximum);O->SetBoolField(TEXT("exhausted"),D->Stamina->bExhausted);O->SetBoolField(TEXT("sprinting"),D->bSprinting);
    O->SetNumberField(TEXT("combo"),D->Combat->ComboCount);O->SetNumberField(TEXT("attackSerial"),D->Combat->AttackSerial);O->SetNumberField(TEXT("attackElapsed"),D->Combat->AttackElapsed);O->SetBoolField(TEXT("weakAttack"),D->Combat->bWeakAttack);
    O->SetNumberField(TEXT("damageDealt"),D->Combat->LastDealtDamage);
    O->SetNumberField(TEXT("targetHealth"),IsValid(TestTarget)?TestTarget->Health->Current:-1);
    O->SetNumberField(TEXT("meanFPS"),FrameSum>0?FrameCount/FrameSum:0);
    O->SetNumberField(TEXT("frameCount"),FrameCount);O->SetNumberField(TEXT("frameSeconds"),FrameSum);
    int32 Major=0;for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor)Major++;
    O->SetNumberField(TEXT("majorCount"),Major);O->SetNumberField(TEXT("respawnDelay"),D->RespawnDelay);
    if(auto* GM=GetWorld()->GetGameState<ADinoGameState>())
    {
        O->SetBoolField(TEXT("lobby"),GM->bLobby);O->SetBoolField(TEXT("bots"),GM->bFillBots);O->SetNumberField(TEXT("capacity"),GM->MaxParticipants);O->SetNumberField(TEXT("round"),GM->RoundNumber);
        TArray<TSharedPtr<FJsonValue>> Players;for(auto P:GM->PlayerArray)if(auto* PS=Cast<ADinoPlayerState>(P)){auto R=MakeShared<FJsonObject>();R->SetNumberField(TEXT("id"),PS->CombatantID);R->SetNumberField(TEXT("species"),PS->SelectedSpecies);R->SetNumberField(TEXT("team"),PS->TeamID);R->SetBoolField(TEXT("ready"),PS->bReady);R->SetBoolField(TEXT("host"),PS->bHost);R->SetStringField(TEXT("name"),PS->GetPlayerName());Players.Add(MakeShared<FJsonValueObject>(R));}O->SetArrayField(TEXT("players"),Players);O->SetStringField(TEXT("lobbyStatus"),LobbyStatus);
        auto Score=GM->GetScore(D->CombatantID);O->SetNumberField(TEXT("kills"),Score.Kills);O->SetNumberField(TEXT("deaths"),Score.Deaths);O->SetNumberField(TEXT("assists"),Score.Assists);
        O->SetBoolField(TEXT("teamMode"),GM->bTeamMatch);O->SetBoolField(TEXT("roundOver"),GM->bRoundOver);O->SetNumberField(TEXT("winner"),GM->WinnerID);O->SetNumberField(TEXT("winnerTeam"),GM->WinnerTeam);
        O->SetNumberField(TEXT("team0Kills"),GM->TeamKills[0]);O->SetNumberField(TEXT("team1Kills"),GM->TeamKills[1]);O->SetNumberField(TEXT("goal"),GM->bTeamMatch?GM->TeamKillGoal:GM->SoloKillGoal);
        TArray<TSharedPtr<FJsonValue>> Board;for(auto Pair:GM->Scores){auto Row=MakeShared<FJsonObject>();Row->SetNumberField(TEXT("id"),Pair.Key);Row->SetNumberField(TEXT("kills"),Pair.Value.Kills);Row->SetNumberField(TEXT("deaths"),Pair.Value.Deaths);Row->SetNumberField(TEXT("assists"),Pair.Value.Assists);Board.Add(MakeShared<FJsonValueObject>(Row));}O->SetArrayField(TEXT("scoreboard"),Board);
    }
    for(TActorIterator<ADinoEffects> It(GetWorld());It;++It){O->SetNumberField(TEXT("bloodParticles"),It->ActiveCount());O->SetNumberField(TEXT("bloodEmitted"),It->TotalEmitted);}
    O->SetBoolField(TEXT("showNameTags"),bShowNameTags);O->SetBoolField(TEXT("settingsOpen"),bSettingsOpen);O->SetBoolField(TEXT("bloodEnabled"),bBloodEnabled);O->SetBoolField(TEXT("inWater"),D->bInWater);O->SetBoolField(TEXT("menuOpen"),bSelectionOpen);O->SetBoolField(TEXT("mapOpen"),bMapOpen);O->SetNumberField(TEXT("sensitivity"),D->MouseSensitivity);O->SetStringField(TEXT("animation"),D->Animation->State);O->SetBoolField(TEXT("eating"),D->Food->bEating);O->SetNumberField(TEXT("foodConsumed"),D->Food->FoodConsumed);
    TArray<TSharedPtr<FJsonValue>> Materials;
    for(int32 I=0;I<D->GetMesh()->GetNumMaterials();++I)if(auto* M=D->GetMesh()->GetMaterial(I))Materials.Add(MakeShared<FJsonValueString>(GetNameSafe(M->GetMaterial())));
    O->SetArrayField(TEXT("materials"),Materials);
    O->SetStringField(TEXT("region"),ALostValleyWorld::RegionName(L));
    TArray<TSharedPtr<FJsonValue>> AIs;
    for(TActorIterator<ADinosaurAIController> It(GetWorld());It;++It)
    {
        auto* Other=Cast<ADinosaurCharacter>(It->GetPawn());if(!Other)continue;
        auto Row=MakeShared<FJsonObject>();Row->SetNumberField(TEXT("team"),Other->TeamID);if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())Row->SetBoolField(TEXT("scoringTarget"),GM->IsScoringTarget(Other));Row->SetNumberField(TEXT("id"),Other->CombatantID);Row->SetNumberField(TEXT("species"),Other->Species);Row->SetBoolField(TEXT("major"),Other->bMajor);
        FVector Marker=FVector::ZeroVector;Row->SetBoolField(TEXT("inSight"),D->CanSeeDinosaur(Other));Row->SetBoolField(TEXT("mapVisible"),D->MapPositionFor(Other,Marker));Row->SetNumberField(TEXT("markerX"),Marker.X);Row->SetNumberField(TEXT("markerY"),Marker.Y);Row->SetNumberField(TEXT("revealUntil"),Other->RevealUntil);Row->SetNumberField(TEXT("hunger"),Other->Hunger->Current);
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
    TArray<TSharedPtr<FJsonValue>> Corpses,Plants;
    for(TActorIterator<ADinosaurCarcass> It(GetWorld());It;++It){auto R=MakeShared<FJsonObject>();R->SetStringField(TEXT("name"),It->GetName());R->SetNumberField(TEXT("species"),It->Species);R->SetNumberField(TEXT("source"),It->SourceID);R->SetNumberField(TEXT("z"),It->GetActorLocation().Z);R->SetNumberField(TEXT("bodyZ"),It->Body->GetComponentLocation().Z);R->SetNumberField(TEXT("bodyRelativeZ"),It->Body->GetRelativeLocation().Z);R->SetNumberField(TEXT("ground"),ALostValleyWorld::HeightAt(It->GetActorLocation().X,It->GetActorLocation().Y));R->SetNumberField(TEXT("spineZ"),It->Body->GetSocketLocation(TEXT("spine")).Z);R->SetNumberField(TEXT("food"),It->Nutrition);R->SetNumberField(TEXT("maxFood"),It->MaximumNutrition);R->SetBoolField(TEXT("frozen"),!It->Body->IsComponentTickEnabled());R->SetNumberField(TEXT("x"),It->GetActorLocation().X);R->SetNumberField(TEXT("y"),It->GetActorLocation().Y);Corpses.Add(MakeShared<FJsonValueObject>(R));}
    for(TActorIterator<AFoodPlant> It(GetWorld());It;++It){auto R=MakeShared<FJsonObject>();R->SetStringField(TEXT("name"),It->GetName());R->SetNumberField(TEXT("food"),It->Nutrition);R->SetBoolField(TEXT("hidden"),It->IsHidden());R->SetBoolField(TEXT("outline"),It->Visual->bRenderCustomDepth);R->SetNumberField(TEXT("x"),It->GetActorLocation().X);R->SetNumberField(TEXT("y"),It->GetActorLocation().Y);Plants.Add(MakeShared<FJsonValueObject>(R));}
    O->SetArrayField(TEXT("corpses"),Corpses);O->SetArrayField(TEXT("plants"),Plants);
    FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(O,W);
    FFileHelper::SaveStringToFile(Out,*(BridgeRoot/TEXT("telemetry.json")));
}

