#pragma once
#include "CoreMinimal.h"

/** All balance values are read from [Dino.Trex/Raptor/Trike/Prey] in DefaultGame.ini. */
struct FSpeciesData
{
    FString Name, AssetName;
    float MaxHealth=1000, Damage=90, Speed=1050, Acceleration=1800, TurnRate=130;
    float Radius=95, HalfHeight=190, CameraDistance=1350, CameraHeight=220;
    float AttackRange=630, AttackWidth=180, Recovery=.7f, Windup=.14f;
    float ChargeTime=1.5f, ChargeMultiplier=2.6f, ChargeRecovery=1.25f, LungeSpeed=1200;
    float BraceMultiplier=.2f, JumpVelocity=650, RegenDelay=5, RegenRate=.02f;
    float AIEngageConfidence=.40f,AIRetreatConfidence=.28f,AIGuardCooldown=1.2f,AISupportRadius=3000;
    FLinearColor Color=FLinearColor(.27f,.35f,.19f);
    static const FSpeciesData& Get(int32 Species);
};
