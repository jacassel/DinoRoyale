#include "DinosaurAIController.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "HungerComponent.h"
#include "CombatComponent.h"
#include "FoodSystem.h"
#include "LostValleyWorld.h"
#include "DinoGameMode.h"
#include "DinoTactics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
ADinosaurAIController::ADinosaurAIController(){PrimaryActorTick.bCanEverTick=true;bAttachToPawn=true;}
ADinosaurCharacter* ADinosaurAIController::Dino() const{return Cast<ADinosaurCharacter>(GetPawn());}
void ADinosaurAIController::OnPossess(APawn* P)
{
    Super::OnPossess(P);for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
    auto* D=Dino();if(D){Random.Initialize(9137+D->CombatantID*71);Personality=D->CombatantID%4;Temperament=Random.FRandRange(.85f,1.15f);LastLocation=LastProgress=D->GetActorLocation();D->GetCharacterMovement()->bOrientRotationToMovement=false;}
    ThinkTimer=Random.FRandRange(.05f,.3f);
}
ADinosaurCharacter* ADinosaurAIController::PackLeader() const
{
    if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())return GM->GetPackLeader(Dino());
    return nullptr;
}
ADinosaurCharacter* ADinosaurAIController::SelectEnemy() const
{
    auto* D=Dino();if(!D)return nullptr;
    ADinosaurCharacter* Best=nullptr;ADinosaurCharacter* BestMajor=nullptr;float Score=FLT_MAX,MajorScore=FLT_MAX;bool MajorThreat=false;
    const float Now=GetWorld()->GetTimeSeconds();
    float Range=D->Species==2?6500:D->Species==3?4500:14000;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* O=*It;if(!D->IsEnemy(O))continue;
        if(D->Species==2&&O->Species==3)continue;
        if(D->Species==3&&(O->Species==2||O->Species==3))continue;
        float Dist=FVector::Dist2D(D->GetActorLocation(),O->GetActorLocation());if(Dist>Range)continue;
        if(D->Species!=3&&O->Species==3)
        {
            const float HuntRange=Personality==0?2400.f:Personality==2?3800.f:3200.f;
            if(D->Hunger->Fraction()>.70f||Now<PreyCooldownUntil||Dist>HuntRange||FMath::Max(FMath::Abs(O->GetActorLocation().X),FMath::Abs(O->GetActorLocation().Y))>24500||!Valley||!Valley->IsWalkable(O->GetActorLocation(),D->Stats().Radius))continue;
        }
        auto Assessment=AssessDinosaurFight(D,O);
        if(D->Species!=3&&O->bMajor)
        {
            const bool HelpsAlly=Assessment.Allies>0&&(O->Combat->bCharging||O->Combat->IsBusy());
            MajorThreat|=Dist<(Personality==1?4000.f:3200.f)||Assessment.bFinishingOpportunity||HelpsAlly||O==RecentAttacker.Get();
        }
        if(D->Species!=3&&Assessment.FightConfidence<D->Stats().AIEngageConfidence&&Dist>4000&&O!=RecentAttacker.Get())continue;
        float S=(Dist+650)*(1.65f-Assessment.FightConfidence);
        if(D->Species!=3&&D->Species!=2)S*=O->Species==3?(D->Hunger->Fraction()<.65f?.35f:1.7f):.7f;
        if(D->Species==2&&O->Species==0)S*=.8f;
        if(O->bMajor&&S<MajorScore){BestMajor=O;MajorScore=S;}
        if(S<Score){Best=O;Score=S;}
    }
    return MajorThreat&&BestMajor?BestMajor:Best;
}
void ADinosaurAIController::Alert(ADinosaurCharacter* A)
{
    if(A&&Dino()&&Dino()->IsEnemy(A))
    {
        // Finish the current close-range response instead of spinning toward every pack hit.
        if(Target.IsValid()&&!Target->bDead&&Target.Get()!=A&&Target->Species!=3&&FVector::Dist2D(Dino()->GetActorLocation(),Target->GetActorLocation())<Dino()->Stats().AttackRange*1.6f)return;
        if(RecentAttacker.Get()!=A)++Retaliations;
        RecentAttacker=A;Target=A;if(A->bMajor){PreyChaseStarted=-1;PreyCooldownUntil=GetWorld()->GetTimeSeconds()+10;}RetaliationUntil=GetWorld()->GetTimeSeconds()+8;ThinkTimer=0;
    }
}
void ADinosaurAIController::ChooseRetreat(const ADinosaurCharacter* Threat)
{
    auto* D=Dino();if(!D||!Threat||!Valley)return;
    FVector P=D->GetActorLocation(),Away=(P-Threat->GetActorLocation()).GetSafeNormal2D();
    FVector Best=P+Away*4500;float BestScore=-FLT_MAX;
    for(float Angle:{0.f,-35.f,35.f,-70.f,70.f})
    {
        FVector Candidate=Valley->NearestWalkable(P+Away.RotateAngleAxis(Angle,FVector::UpVector)*4500);
        if(!Valley->IsWalkable(Candidate,D->Stats().Radius+80))continue;
        float Score=FVector::Dist2D(Candidate,Threat->GetActorLocation())-FVector::Dist2D(P,Candidate)*.12f;
        if(Leader.IsValid()&&!Leader->bDead&&Leader.Get()!=D)Score-=FVector::Dist2D(Candidate,Leader->GetActorLocation())*.20f;
        if(Score>BestScore){BestScore=Score;Best=Candidate;}
    }
    GoTo(Best);RetreatUntil=GetWorld()->GetTimeSeconds()+2.2f;++RetreatDecisions;
}
void ADinosaurAIController::SetTravelGoal(const FVector& Point){bForcedTravel=true;Target=nullptr;ForcedDestination=Valley?Valley->NearestWalkable(Point):Point;GoTo(ForcedDestination);State=TEXT("Traversing");}
void ADinosaurAIController::ClearTravelGoal(){bForcedTravel=false;Path.Empty();RoamTimer=0;}
void ADinosaurAIController::ResetTactics(){PreyChaseStarted=-1;PreyCooldownUntil=0;Target=nullptr;Leader=nullptr;RecentAttacker=nullptr;RetaliationUntil=RetreatUntil=NextGuardTime=NextTargetReview=BraceTime=RepositionUntil=NextReposition=0;FightConfidence=EscapeConfidence=.5f;Decision=TEXT("Explore");}
void ADinosaurAIController::GoTo(const FVector& Point)
{
    if(!Valley||!Dino())return;
    FVector Dest=Valley->NearestWalkable(Point);
    if(PathTimer>0&&FVector::DistSquared2D(Dest,Goal)<FMath::Square(800.f)&&PathIndex<Path.Num())return;
    Goal=Dest;PathIndex=0;PathTimer=.7f;
    ++Valley->PathRequests;
    if(!Valley->FindPath(Dino()->GetActorLocation(),Goal,Path)){++FailedPaths;++Valley->PathFailures;Path.Empty();}
}
void ADinosaurAIController::Tick(float Dt)
{
    Super::Tick(Dt);auto* D=Dino();if(!D||bPaused||D->MatchFrozen())return;
    float Travel=FVector::Dist2D(D->GetActorLocation(),LastLocation);if(Travel<5000)DistanceTravelled+=Travel;LastLocation=D->GetActorLocation();
    if(D->bDead){Path.Empty();Target=nullptr;State=TEXT("Dead");return;}
    ThinkTimer-=Dt;PathTimer-=Dt;RoamTimer-=Dt;BraceTime-=Dt;
    if(ThinkTimer<=0){Think(.22f);ThinkTimer=.22f;}
    Steer(Dt);
}
void ADinosaurAIController::Think(float Dt)
{
    auto* D=Dino();if(!D||!Valley)return;
    const float Now=GetWorld()->GetTimeSeconds();D->SprintOff();
    if(bForcedTravel){if(PathIndex>=Path.Num()&&FVector::Dist2D(D->GetActorLocation(),ForcedDestination)>240)GoTo(ForcedDestination);return;}
    if(BraceTime<=0&&D->Combat->bBracing)D->Combat->SetBrace(false);
    if(Target.IsValid()&&(Target->bDead||FVector::Dist2D(Target->GetActorLocation(),D->GetActorLocation())>(D->Species==2?6500:12000)))Target=nullptr;
    if(D->Species==1)
    {
        Leader=PackLeader();
        if(Leader.IsValid()&&Leader.Get()!=D)
        {
            if(FVector::Dist2D(D->GetActorLocation(),Leader->GetActorLocation())>5000&&Now>RetaliationUntil)
            {
                Target=nullptr;State=TEXT("Regrouping");GoTo(Leader->GetActorLocation()-Leader->GetActorForwardVector()*800+FVector(0,(D->CombatantID%3-1)*600,0));return;
            }
            if(auto* LC=Cast<ADinosaurAIController>(Leader->GetController()))if(LC->Target.IsValid())Target=LC->Target;
            if(Leader->IsPlayerControlled()&&Leader->LastAttacker.IsValid()&&!Leader->LastAttacker->bDead)Target=Leader->LastAttacker;
        }
    }
    if(D->Species!=3&&Target.IsValid()&&Target->Species==3)
    {
        if(PreyChaseStarted<0){PreyChaseStarted=Now;PreyChaseOrigin=D->GetActorLocation();}
        const float Limit=Personality==0?4.f:Personality==2?8.f:6.f;
        const FVector PreyPosition=Target->GetActorLocation();
        if(Now-PreyChaseStarted>Limit||FVector::Dist2D(PreyChaseOrigin,D->GetActorLocation())>3500||FMath::Max(FMath::Abs(PreyPosition.X),FMath::Abs(PreyPosition.Y))>24500||D->Hunger->Fraction()>.75f)
        {Target=nullptr;Path.Empty();PreyChaseStarted=-1;PreyCooldownUntil=Now+12;NextTargetReview=0;++AbandonedPreyChases;}
    }
    else PreyChaseStarted=-1;
    if(RecentAttacker.IsValid()&&!RecentAttacker->bDead&&Now<RetaliationUntil&&FVector::Dist2D(D->GetActorLocation(),RecentAttacker->GetActorLocation())<12000)Target=RecentAttacker;
    else if(!Target.IsValid()||Now>NextTargetReview)
    {
        ADinosaurCharacter* Candidate=nullptr;
        if(Leader.IsValid()&&Leader.Get()!=D)if(auto* LC=Cast<ADinosaurAIController>(Leader->GetController()))if(LC->Target.IsValid()&&LC->Target->bMajor)Candidate=LC->Target.Get();
        if(!Candidate)Candidate=SelectEnemy();NextTargetReview=Now+1.2f;
        if(Candidate&&(!Target.IsValid()||Target->Species==3||FVector::Dist2D(D->GetActorLocation(),Target->GetActorLocation())>2200))Target=Candidate;
    }
    if(D->Species==1&&Leader.IsValid()&&Leader->IsPlayerControlled()&&Target.IsValid()&&Now>RetaliationUntil&&FVector::Dist2D(Target->GetActorLocation(),Leader->GetActorLocation())>5000)Target=nullptr;
    FVector Position=D->GetActorLocation();float Fraction=D->Health->Fraction();
    if(D->Species!=3&&D->Hunger->Fraction()<.65f&&!D->Combat->IsBusy()&&!D->Combat->bCharging&&Now>RetaliationUntil)
    {
        const bool ImmediateThreat=Target.IsValid()&&FVector::Dist2D(Position,Target->GetActorLocation())<3200&&Target->Species!=3;
        if(!ImmediateThreat&&D->Food->bEating){State=TEXT("Feeding");Path.Empty();return;}
        if(!ImmediateThreat)if(auto* Meal=D->Food->FindFood(18000))
        {
            Target=nullptr;D->Combat->SetBrace(false);State=TEXT("Seeking food");Decision=TEXT("Replenish hunger and stamina");GoTo(Meal->GetActorLocation());
            if(FVector::Dist2D(Position,Meal->GetActorLocation())<D->Stats().AttackRange+200){Path.Empty();D->Food->StartEating();State=D->Food->bEating?TEXT("Feeding"):TEXT("Seeking food");}return;
        }
    }
    if(D->Species==3)
    {
        if(Target.IsValid())
        {
            State=TEXT("Fleeing");FVector Away=(Position-Target->GetActorLocation()).GetSafeNormal2D();GoTo(Position+Away*6500);return;
        }
    }
    else if(Target.IsValid())
    {
        auto* Enemy=Target.Get();auto A=AssessDinosaurFight(D,Enemy);FightConfidence=A.FightConfidence;EscapeConfidence=A.EscapeConfidence;
        float Dist=FVector::Dist2D(Position,Enemy->GetActorLocation());FVector ToEnemy=(Enemy->GetActorLocation()-Position).GetSafeNormal2D();
        bool EnemyFacing=FVector::DotProduct(Enemy->GetActorForwardVector(),-ToEnemy)>.40f;
        bool HeavyThreat=Enemy->Combat->bCharging&&Enemy->Combat->ChargeFraction()>.18f;
        bool FreshAttack=Enemy->Combat->IsBusy()&&Enemy->Combat->AttackElapsed<.28f;
        bool Desperate=FightConfidence<.35f&&EscapeConfidence<.38f;
        if(Dist<Enemy->Stats().AttackRange+D->Stats().Radius+220&&EnemyFacing&&(HeavyThreat||FreshAttack||Desperate)&&Now>=NextGuardTime&&FVector::DotProduct(D->GetActorForwardVector(),ToEnemy)>.65f&&!D->Combat->IsBusy()&&!D->Combat->bCharging&&(HeavyThreat||Desperate||Random.FRand()<(Personality==1?.95f:Personality==0?.38f:.65f)*Temperament))
        {
            if(D->Combat->SetBrace(true))
            {
                BraceTime=HeavyThreat?FMath::Clamp(Enemy->Stats().ChargeTime*(1-Enemy->Combat->ChargeFraction())+Enemy->Stats().HeavyWindup/Enemy->Health->AttackSpeedFactor()+.25f,.5f,1.8f):.60f;
                NextGuardTime=Now+BraceTime+D->Stats().AIGuardCooldown;++GuardsUsed;Path.Empty();
            }
        }
        if(D->Combat->bBracing){State=TEXT("Bracing");Decision=HeavyThreat?TEXT("Block incoming heavy attack"):TEXT("Guard and counter");return;}
        // Recover resources or create an opening, with a committed short path to avoid indecisive spinning.
        const bool Tired=D->Stamina->Fraction()<.25f||D->Stamina->bExhausted;
        const bool WantsSpace=Tired||(Personality==2&&Enemy->Combat->IsBusy()&&Dist<Enemy->Stats().AttackRange*1.8f);
        if(!D->Combat->IsBusy()&&!D->Combat->bCharging&&!A.bFinishingOpportunity&&
           (Now<RepositionUntil||(Now>NextReposition&&WantsSpace&&Dist<Enemy->Stats().AttackRange*2.3f)))
        {
            if(Now>=RepositionUntil)
            {
                FVector Away=(Position-Enemy->GetActorLocation()).GetSafeNormal2D();
                FVector Side=FVector::CrossProduct(Away,FVector::UpVector)*(D->CombatantID%2?1:-1);
                GoTo(Position+(Away+Side*.45f).GetSafeNormal()*1800);
                RepositionUntil=Now+(Tired?2.0f:1.1f);NextReposition=RepositionUntil+2.0f;
            }
            State=TEXT("Repositioning");Decision=Tired?TEXT("Recover stamina before re-engaging"):TEXT("Skirmish around committed attack");
            if(!Tired&&D->Stamina->Fraction()>.4f)D->SprintOn();return;
        }
        bool Losing=FightConfidence<D->Stats().AIRetreatConfidence||(Fraction<.25f&&FightConfidence<.55f)||Now<RetreatUntil;
        if(!A.bFinishingOpportunity&&Losing&&EscapeConfidence>FightConfidence+.06f&&EscapeConfidence>.33f)
        {
            D->Combat->bCharging=false;D->Food->StopEating();
            if(State!=TEXT("Retreating")||Now>=RetreatUntil)ChooseRetreat(Enemy);
            State=TEXT("Retreating");Decision=TEXT("Escape has better survival odds");if(D->Stamina->Fraction()>.28f)D->SprintOn();return;
        }
        Decision=A.bFinishingOpportunity?TEXT("Finish vulnerable target"):Now<RetaliationUntil?TEXT("Retaliate against attacker"):FightConfidence>D->Stats().AIEngageConfidence?TEXT("Favorable fight"):TEXT("Stand ground and counter");
    }
    if(D->Combat->bCharging)
    {
        if(!Target.IsValid()){D->Combat->bCharging=false;return;}
        if(D->Combat->ChargeFraction()>=ChargeTarget){D->Combat->ReleaseCharge();++AttacksMade;}
        State=TEXT("Charging");return;
    }
    if(D->Food->bEating){State=TEXT("Feeding");Path.Empty();if(Target.IsValid()&&FVector::Dist2D(Position,Target->GetActorLocation())<1800)D->Food->StopEating();else return;}
    if(Target.IsValid()&&D->Species!=3)
    {
        auto* Enemy=Target.Get();float Dist=FVector::Dist2D(Position,Enemy->GetActorLocation());
        State=TEXT("Pursuing");FVector ToEnemy=(Enemy->GetActorLocation()-Position).GetSafeNormal2D();
        float AttackDistance=D->Stats().AttackRange+Enemy->Stats().Radius*.25f;
        if(D->Species==1&&Enemy->Combat->bBracing&&Dist<AttackDistance*2.5f&&FVector::DotProduct(Enemy->GetActorForwardVector(),-ToEnemy)>.2f)
        {
            FVector Side=FVector::CrossProduct(Enemy->GetActorForwardVector(),FVector::UpVector)*(D->CombatantID%2?1:-1);
            GoTo(Enemy->GetActorLocation()+Side*AttackDistance*1.15f-Enemy->GetActorForwardVector()*AttackDistance*.35f);State=TEXT("Flanking");Decision=TEXT("Go around frontal guard");return;
        }
        if(Dist<AttackDistance*.83f)
        {
            Path.Empty();State=TEXT("Attacking");
            if(FVector::DotProduct(D->GetActorForwardVector(),ToEnemy)>.75f&&!D->Combat->IsBusy()&&!D->Combat->bBracing)
            {
                bool Finish=Enemy->Health->Current<=D->Stats().Damage*1.05f;
                if(!Finish&&Random.FRand()<FMath::Clamp((Personality==0?.18f:Personality==1?.45f:Personality==2?.38f:.52f)*Temperament+(Enemy->Stamina->bExhausted||Enemy->Combat->RecoveryLeft>.5f?.25f:0.f),.1f,.85f)&&D->Combat->StartCharge()){ChargeTarget=Random.FRandRange(.75f,1.f);State=TEXT("Charging");}
                else if(D->Combat->QuickAttack())++AttacksMade;
            }
        }
        else
        {
            FVector Dest=Enemy->GetActorLocation();
            if(D->Species==1&&Dist>AttackDistance*1.2f&&Dist<3200)
            {
                float A=(D->CombatantID%4)*PI*.5f+GetWorld()->GetTimeSeconds()*.02f;
                Dest+=FVector(FMath::Cos(A),FMath::Sin(A),0)*(AttackDistance*.58f);
            }
            if(Dist>AttackDistance*2&&Dist<7000&&D->Stamina->Fraction()>(Personality==2?.4f:.6f))D->SprintOn();
            GoTo(Dest);
        }
        return;
    }
    if(Fraction<.88f||D->Stamina->Fraction()<.45f||D->Hunger->Fraction()<.7f)if(auto* Food=D->Food->FindFood(12000))
    {
        State=TEXT("Seeking food");GoTo(Food->GetActorLocation());
        if(FVector::Dist2D(Position,Food->GetActorLocation())<D->Stats().AttackRange+200){Path.Empty();D->Food->StartEating();}return;
    }
    if(D->Species==1&&Leader.IsValid()&&Leader.Get()!=D)
    {
        float A=(D->CombatantID%5)*2.4f;FVector Slot=Leader->GetActorLocation()-Leader->GetActorForwardVector()*650+FVector(FMath::Cos(A),FMath::Sin(A),0)*500;
        if(FVector::Dist2D(Position,Slot)>420){State=TEXT("Following pack");GoTo(Slot);}else{State=TEXT("With pack");Path.Empty();}
        return;
    }
    State=TEXT("Roaming");
    Decision=Fraction<.5f?TEXT("Recover away from danger"):TEXT("Explore and locate opponents");
    if(RoamTimer<=0||PathIndex>=Path.Num())
    {
        FVector Home=D->HomePosition;
        if(D->Species==2&&!Valley->FeedingSpots.IsEmpty())Home=Valley->FeedingSpots[D->CombatantID%Valley->FeedingSpots.Num()];
        if(D->Species==0&&Random.FRand()<.35f)Home=ALostValleyWorld::Landmarks()[Random.RandRange(0,5)];
        if(auto* GM=GetWorld()->GetAuthGameMode<ADinoGameMode>())if(GM->bTeamMatch&&D->bMajor)Home=FVector(D->TeamID==0?-1800:1800,0,0);
        GoTo(Home+FVector(Random.FRandRange(-9000,9000),Random.FRandRange(-9000,9000),0));RoamTimer=Random.FRandRange(12,25);
    }
}
void ADinosaurAIController::Steer(float Dt)
{
    auto* D=Dino();if(!D||!Valley)return;
    FVector P=D->GetActorLocation(),Facing=FVector::ZeroVector,Direction=FVector::ZeroVector;
    if(Target.IsValid()&&D->Species!=3&&!bForcedTravel&&State!=TEXT("Retreating")&&State!=TEXT("Repositioning"))Facing=(Target->GetActorLocation()-P).GetSafeNormal2D();
    if(D->Combat->bBracing||D->Food->bEating){D->GetCharacterMovement()->StopMovementImmediately();return;}
    while(PathIndex<Path.Num()&&FVector::Dist2D(P,Path[PathIndex])<(Target.IsValid()?90.f:240.f))++PathIndex;
    if(PathIndex<Path.Num())
    {
        Direction=(Path[PathIndex]-P).GetSafeNormal2D();
        Direction=Valley->AvoidObstacles(P,Direction,D->Stats().Radius);
        FVector Separation=FVector::ZeroVector;
        for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
        {
            auto* O=*It;if(O==D||O->bDead)continue;FVector Offset=P-O->GetActorLocation();Offset.Z=0;float Dist=Offset.Size();
            float Desired=D->Stats().Radius+O->Stats().Radius+170;
            if(Dist>1&&Dist<Desired&&O!=Target.Get())Separation+=Offset/Dist*((Desired-Dist)/Desired)*(O->IsPlayerControlled()?2.2f:1.f);
        }
        Direction=(Direction+Separation).GetSafeNormal2D();
        if(Facing.IsNearlyZero()||FVector::Dist2D(P,Goal)>2500||D->Species==3)Facing=Direction;
        D->AddMovementInput(Direction,D->Combat->bCharging?.55f:1.f);
        if(FVector::Dist2D(P,LastProgress)<80)StuckTime+=Dt;
        else{LastProgress=P;StuckTime=0;}
        if(StuckTime>2.5f)
        {
            ++StuckRecoveries;StuckTime=0;LastProgress=P;PathTimer=0;
            FVector Nudge=Valley->NearestWalkable(P+FVector(Random.FRandRange(-900,900),Random.FRandRange(-900,900),0));
            if(Valley->SegmentClear(P,Nudge,D->Stats().Radius+30))GoTo(Nudge);
            else {GoTo(Goal);D->BeginJump();}
        }
    }
    else StuckTime=0;
    if(!Facing.IsNearlyZero())D->SetActorRotation(FMath::RInterpConstantTo(D->GetActorRotation(),FRotator(0,Facing.Rotation().Yaw,0),Dt,D->Stats().TurnRate*D->TurnFactor()));
}
