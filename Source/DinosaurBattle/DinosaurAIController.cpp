#include "DinosaurAIController.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "FoodSystem.h"
#include "LostValleyWorld.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
ADinosaurAIController::ADinosaurAIController(){PrimaryActorTick.bCanEverTick=true;bAttachToPawn=true;}
ADinosaurCharacter* ADinosaurAIController::Dino() const{return Cast<ADinosaurCharacter>(GetPawn());}
void ADinosaurAIController::OnPossess(APawn* P)
{
    Super::OnPossess(P);for(TActorIterator<ALostValleyWorld> It(GetWorld());It;++It){Valley=*It;break;}
    auto* D=Dino();if(D){Random.Initialize(9137+D->CombatantID*71);LastLocation=LastProgress=D->GetActorLocation();D->GetCharacterMovement()->bOrientRotationToMovement=false;}
    ThinkTimer=Random.FRandRange(.05f,.3f);
}
ADinosaurCharacter* ADinosaurAIController::PackLeader() const
{
    ADinosaurCharacter* Best=nullptr;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
        if(It->Species==1&&!It->bDead&&(!Best||It->IsPlayerControlled()||(!Best->IsPlayerControlled()&&It->CombatantID<Best->CombatantID)))Best=*It;
    return Best;
}
ADinosaurCharacter* ADinosaurAIController::SelectEnemy() const
{
    auto* D=Dino();if(!D)return nullptr;
    ADinosaurCharacter* Best=nullptr;float Score=FLT_MAX;
    float Range=D->Species==2?6500:D->Species==3?4500:14000;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)
    {
        auto* O=*It;if(!D->IsEnemy(O))continue;
        if(D->Species==2&&O->Species==3)continue;
        if(D->Species==3&&(O->Species==2||O->Species==3))continue;
        float Dist=FVector::Dist2D(D->GetActorLocation(),O->GetActorLocation());if(Dist>Range)continue;
        float S=Dist;
        if(D->Species!=3&&D->Species!=2)S*=O->Species==3?1.7f:.7f;
        if(D->Species==2&&O->Species==0)S*=.8f;
        if(S<Score){Best=O;Score=S;}
    }
    return Best;
}
void ADinosaurAIController::Alert(ADinosaurCharacter* A){if(A&&Dino()&&Dino()->IsEnemy(A))Target=A;}
void ADinosaurAIController::SetTravelGoal(const FVector& Point){bForcedTravel=true;Target=nullptr;GoTo(Point);State=TEXT("Traversing");}
void ADinosaurAIController::ClearTravelGoal(){bForcedTravel=false;Path.Empty();RoamTimer=0;}
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
    Super::Tick(Dt);auto* D=Dino();if(!D||bPaused)return;
    float Travel=FVector::Dist2D(D->GetActorLocation(),LastLocation);if(Travel<5000)DistanceTravelled+=Travel;LastLocation=D->GetActorLocation();
    if(D->bDead){Path.Empty();Target=nullptr;State=TEXT("Dead");return;}
    ThinkTimer-=Dt;PathTimer-=Dt;RoamTimer-=Dt;BraceTime-=Dt;
    if(ThinkTimer<=0){Think(.22f);ThinkTimer=.22f;}
    Steer(Dt);
}
void ADinosaurAIController::Think(float Dt)
{
    auto* D=Dino();if(!D||!Valley)return;
    if(bForcedTravel){if(PathIndex>=Path.Num()&&FVector::Dist2D(D->GetActorLocation(),Goal)>500)GoTo(Goal);return;}
    if(BraceTime<=0&&D->Combat->bBracing)D->Combat->SetBrace(false);
    if(D->Combat->bCharging)
    {
        if(!Target.IsValid()||D->Combat->ChargeFraction()>=ChargeTarget){D->Combat->ReleaseCharge();++AttacksMade;}
        return;
    }
    if(Target.IsValid()&&(Target->bDead||FVector::Dist2D(Target->GetActorLocation(),D->GetActorLocation())>(D->Species==2?6500:12000)))Target=nullptr;
    if(D->Species==1)
    {
        Leader=PackLeader();
        if(Leader.IsValid()&&Leader.Get()!=D)
        {
            if(FVector::Dist2D(D->GetActorLocation(),Leader->GetActorLocation())>5000)
            {
                Target=nullptr;State=TEXT("Regrouping");GoTo(Leader->GetActorLocation()-Leader->GetActorForwardVector()*800+FVector(0,(D->CombatantID%3-1)*600,0));return;
            }
            if(auto* LC=Cast<ADinosaurAIController>(Leader->GetController()))if(LC->Target.IsValid())Target=LC->Target;
            if(Leader->IsPlayerControlled()&&Leader->LastAttacker.IsValid()&&!Leader->LastAttacker->bDead)Target=Leader->LastAttacker;
        }
    }
    if(!Target.IsValid())Target=SelectEnemy();
    if(D->Species==1&&Leader.IsValid()&&Leader->IsPlayerControlled()&&Target.IsValid()&&FVector::Dist2D(Target->GetActorLocation(),Leader->GetActorLocation())>5000)Target=nullptr;
    FVector Position=D->GetActorLocation();float Fraction=D->Health->Fraction();
    if(D->Species==3)
    {
        if(Target.IsValid())
        {
            State=TEXT("Fleeing");FVector Away=(Position-Target->GetActorLocation()).GetSafeNormal2D();GoTo(Position+Away*6500);return;
        }
    }
    else if(Fraction<.25f)
    {
        D->Combat->SetBrace(false);D->Combat->bCharging=false;
        if(auto* Food=D->Food->FindFood(10000))
        {
            State=TEXT("Seeking food");GoTo(Food->GetActorLocation());
            if(FVector::Dist2D(Position,Food->GetActorLocation())<D->Stats().AttackRange+220){Path.Empty();D->Food->StartEating();}
        }
        else if(Target.IsValid()){State=TEXT("Retreating");GoTo(Position+(Position-Target->GetActorLocation()).GetSafeNormal2D()*6500);}
        else {State=TEXT("Recovering");Path.Empty();}
        return;
    }
    if(D->Food->bEating){State=TEXT("Feeding");Path.Empty();if(Target.IsValid()&&FVector::Dist2D(Position,Target->GetActorLocation())<1800)D->Food->StopEating();else return;}
    if(Target.IsValid()&&D->Species!=3)
    {
        auto* Enemy=Target.Get();float Dist=FVector::Dist2D(Position,Enemy->GetActorLocation());
        State=TEXT("Pursuing");FVector ToEnemy=(Enemy->GetActorLocation()-Position).GetSafeNormal2D();
        float AttackDistance=D->Stats().AttackRange+Enemy->Stats().Radius*.25f;
        if(Dist<AttackDistance*.83f)
        {
            Path.Empty();State=TEXT("Attacking");
            if(FVector::DotProduct(D->GetActorForwardVector(),ToEnemy)>.75f&&!D->Combat->IsBusy()&&!D->Combat->bBracing)
            {
                if(Enemy->Combat->bCharging&&Random.FRand()<.72f){D->Combat->SetBrace(true);BraceTime=.8f;State=TEXT("Bracing");}
                else if(Random.FRand()<(D->Species==0?.42f:.24f))
                {if(D->Combat->StartCharge())ChargeTarget=Random.FRandRange(.45f,.90f);}
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
            GoTo(Dest);
        }
        return;
    }
    if(Fraction<.88f)if(auto* Food=D->Food->FindFood(12000))
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
    if(RoamTimer<=0||PathIndex>=Path.Num())
    {
        FVector Home=D->HomePosition;
        if(D->Species==2&&!Valley->FeedingSpots.IsEmpty())Home=Valley->FeedingSpots[D->CombatantID%Valley->FeedingSpots.Num()];
        if(D->Species==0&&Random.FRand()<.35f)Home=ALostValleyWorld::Landmarks()[Random.RandRange(0,5)];
        GoTo(Home+FVector(Random.FRandRange(-9000,9000),Random.FRandRange(-9000,9000),0));RoamTimer=Random.FRandRange(12,25);
    }
}
void ADinosaurAIController::Steer(float Dt)
{
    auto* D=Dino();if(!D||!Valley)return;
    FVector P=D->GetActorLocation(),Facing=FVector::ZeroVector,Direction=FVector::ZeroVector;
    if(Target.IsValid()&&D->Species!=3&&!bForcedTravel&&State!=TEXT("Retreating"))Facing=(Target->GetActorLocation()-P).GetSafeNormal2D();
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
            else {GoTo(Goal);D->Jump();}
        }
    }
    else StuckTime=0;
    if(!Facing.IsNearlyZero())D->SetActorRotation(FMath::RInterpConstantTo(D->GetActorRotation(),FRotator(0,Facing.Rotation().Yaw,0),Dt,D->Stats().TurnRate));
}
