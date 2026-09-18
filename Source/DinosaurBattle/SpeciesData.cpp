#include "SpeciesData.h"
#include "Misc/ConfigCacheIni.h"

const FSpeciesData& FSpeciesData::Get(int32 Species)
{
    static TArray<FSpeciesData> Data;
    if (Data.IsEmpty())
    {
        const TCHAR* IDs[]={TEXT("Trex"),TEXT("Raptor"),TEXT("Trike"),TEXT("Prey")};
        for (int32 I=0;I<4;++I)
        {
            FSpeciesData D; D.AssetName=IDs[I]; D.Name=IDs[I];
            FString Section=FString(TEXT("Dino."))+IDs[I];
            GConfig->GetString(*Section,TEXT("Name"),D.Name,GGameIni);
            #define READ(V) GConfig->GetFloat(*Section,TEXT(#V),D.V,GGameIni)
            READ(MaxHealth); READ(Damage); READ(Speed); READ(Acceleration); READ(TurnRate);
            READ(Radius); READ(HalfHeight); READ(CameraDistance); READ(CameraHeight);
            READ(AttackRange); READ(AttackWidth); READ(Recovery); READ(Windup);
            READ(ChargeTime); READ(ChargeMultiplier); READ(ChargeRecovery); READ(LungeSpeed);
            READ(BraceMultiplier); READ(JumpVelocity); READ(RegenDelay); READ(RegenRate);
            READ(AIEngageConfidence); READ(AIRetreatConfidence); READ(AIGuardCooldown); READ(AISupportRadius);
            READ(StaminaBusyRegenFactor); READ(WeakAttackDamage); READ(WeakAttackRecovery);
            READ(MaxStamina); READ(StaminaIdleRegen); READ(StaminaWalkRegen); READ(StaminaRegenDelay);
            READ(SprintMultiplier); READ(SprintDrain); READ(SprintTurnFactor); READ(SprintAcceleration);
            READ(JumpCost); READ(QuickCost); READ(HeavyCost); READ(BraceDrain); READ(BraceHitCost); READ(EatStaminaRate);
            READ(ExhaustionDuration); READ(ExhaustionResume); READ(ComboRecovery); READ(ComboReset);
            READ(HeavyWindup); READ(HeavyReach); READ(HeavyDriveTime); READ(HeavyTurnFactor); READ(HeavyKnockback);
            READ(HeavyMissRecovery); READ(HeavyMoveFactor); READ(EatSafeDelay);
            #undef READ
            if(I==1) D.Color=FLinearColor(.15f,.32f,.34f);
            if(I==2) D.Color=FLinearColor(.40f,.22f,.13f);
            if(I==3) D.Color=FLinearColor(.39f,.45f,.18f);
            Data.Add(D);
        }
    }
    return Data[FMath::Clamp(Species,0,3)];
}
