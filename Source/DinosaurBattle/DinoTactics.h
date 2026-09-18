#pragma once
#include "CoreMinimal.h"
class ADinosaurCharacter;
struct FDinoTacticalAssessment
{
    float FightConfidence=.5f,EscapeConfidence=.5f,TimeToKill=10,TimeToDie=10;
    int32 Allies=0,EnemySupport=0;
    bool bFinishingOpportunity=false;
};
/** Bounded local estimates, not a claim to know the outcome of a fight. */
FDinoTacticalAssessment AssessDinosaurFight(const ADinosaurCharacter* Self,const ADinosaurCharacter* Enemy);
