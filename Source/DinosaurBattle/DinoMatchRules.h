#pragma once
#include "CoreMinimal.h"
#include "SpeciesData.h"
#include "DinoMatchRules.generated.h"

/** The same allowed roster and requested bot counts in offline and hosted matches. */
USTRUCT()
struct FDinoMatchRules
{
    GENERATED_BODY()
    UPROPERTY() int32 AllowedSpeciesMask=247; // Stable IDs 0,1,2,4,5,6,7; prey is not a selection.
    UPROPERTY() bool bExplicitBotCounts=false;
    UPROPERTY() int32 TeamABots=4;
    UPROPERTY() int32 TeamBBots=5;
    bool Allows(int32 Species) const {return FSpeciesData::IsPlayable(Species)&&(AllowedSpeciesMask&(1<<Species))!=0;}
    TArray<int32> Allowed() const
    {
        TArray<int32> Result;
        for(int32 I=0;I<FSpeciesData::PlayableCount;++I)if(Allows(FSpeciesData::PlayableID(I)))Result.Add(FSpeciesData::PlayableID(I));
        return Result;
    }
    int32 Resolve(int32 Species) const {const auto IDs=Allowed();return Allows(Species)?Species:IDs.IsEmpty()?0:IDs[0];}
    int32 BotSpecies(int32 Slot) const
    {
        const auto IDs=Allowed();if(IDs.IsEmpty())return 0;
        // Stable random choices keep lobby previews accurate without forcing a
        // perfectly diverse roster. Repeats remain possible in every allowed set.
        FRandomStream Random(719+AllowedSpeciesMask*31);int32 Choice=0;
        for(int32 I=0;I<=FMath::Abs(Slot);++I)Choice=Random.RandHelper(IDs.Num());
        return IDs[Choice];
    }
};
