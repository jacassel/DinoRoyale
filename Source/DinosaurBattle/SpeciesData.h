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
    float MaxStamina=100,StaminaIdleRegen=22,StaminaWalkRegen=14,StaminaRegenDelay=.4f;
    float SprintMultiplier=1.4f,SprintDrain=13,SprintTurnFactor=.7f,SprintAcceleration=1.05f;
    float StaminaBusyRegenFactor=.3f,WeakAttackDamage=.65f,WeakAttackRecovery=1.3f;
    float JumpCost=18,QuickCost=9,HeavyCost=32,BraceDrain=5,BraceHitCost=.05f,EatStaminaRate=32;
    float ExhaustionDuration=1.2f,ExhaustionResume=.25f,ComboRecovery=.8f,ComboReset=1.0f;
    float HeavyWindup=.23f,HeavyReach=1.2f,HeavyDriveTime=.3f,HeavyTurnFactor=.25f,HeavyKnockback=400;
    float HeavyMissRecovery=.4f,HeavyMoveFactor=.25f,EatSafeDelay=2.5f;
    FLinearColor Color=FLinearColor(.27f,.35f,.19f);
    static const FSpeciesData& Get(int32 Species);
};
