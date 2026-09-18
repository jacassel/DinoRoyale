#include "DinoTactics.h"
#include "DinosaurCharacter.h"
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "EngineUtils.h"

FDinoTacticalAssessment AssessDinosaurFight(const ADinosaurCharacter* Self,const ADinosaurCharacter* Enemy)
{
    FDinoTacticalAssessment A;if(!Self||!Enemy)return A;
    auto DPS=[](const ADinosaurCharacter* D){return D->Stats().Damage/FMath::Max(.2f,D->Stats().Recovery)*D->Health->AttackSpeedFactor();};
    float MyDPS=DPS(Self),TheirDPS=DPS(Enemy);
    const float MySpeed=Self->Stats().Speed*Self->Health->MovementFactor(),TheirSpeed=Enemy->Stats().Speed*Enemy->Health->MovementFactor();
    const float Distance=FVector::Dist2D(Self->GetActorLocation(),Enemy->GetActorLocation());
    const bool Frontal=FVector::DotProduct(Enemy->GetActorForwardVector(),(Self->GetActorLocation()-Enemy->GetActorLocation()).GetSafeNormal2D())>.25f;
    float EffectiveHit=Self->Stats().Damage*(Enemy->Combat->bBracing&&Frontal?Enemy->Stats().BraceMultiplier:1.f);
    A.TimeToKill=Enemy->Health->Current/FMath::Max(1.f,MyDPS);A.TimeToDie=Self->Health->Current/FMath::Max(1.f,TheirDPS);
    float Mine=Self->Health->Current*MyDPS,Theirs=Enemy->Health->Current*TheirDPS;
    for(TActorIterator<ADinosaurCharacter> It(Self->GetWorld());It;++It)
    {
        const auto* Other=*It;if(Other==Self||Other==Enemy||Other->bDead||!Other->bMajor)continue;
        float Dist=FVector::Dist2D(Other->GetActorLocation(),Self->GetActorLocation());if(Dist>Self->Stats().AISupportRadius)continue;
        float Weight=.65f*(1-Dist/(Self->Stats().AISupportRadius*1.4f));
        float Power=Other->Health->Current*DPS(Other)*Weight;
        if(!Self->IsEnemy(Other)&&Other->IsEnemy(Enemy)){Mine+=Power;++A.Allies;}
        else if(Self->IsEnemy(Other)&&!Enemy->IsEnemy(Other)){Theirs+=Power;++A.EnemySupport;}
    }
    A.FightConfidence=FMath::Clamp(Mine/FMath::Max(1.f,Mine+Theirs),.02f,.98f);
    A.EscapeConfidence=FMath::Clamp(.40f+(MySpeed-TheirSpeed)/FMath::Max(1.f,TheirSpeed)*.55f+FMath::Clamp(Distance/3000.f,0.f,1.f)*.25f+A.Allies*.06f,.03f,.97f);
    A.bFinishingOpportunity=Enemy->Health->Current<=EffectiveHit*1.05f&&Distance<Self->Stats().AttackRange*1.8f&&A.TimeToDie>Self->Stats().Windup+.1f;
    return A;
}
