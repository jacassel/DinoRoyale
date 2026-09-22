#include "DinoPlayerController.h"
#include "DinosaurCharacter.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

void ADinoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ADinoPlayerController,VisibleMapMarkers,COND_OwnerOnly);
}
void ADinoPlayerController::UpdateMapVisibility()
{
    if(!HasAuthority())return;
    VisibleMapMarkers.Reset();auto* Me=Cast<ADinosaurCharacter>(GetPawn());if(!Me)return;
    for(TActorIterator<ADinosaurCharacter> It(GetWorld());It;++It)if(It->bMajor)
    {
        FVector Position;
        if(Me->MapPositionFor(*It,Position)){FDinoMapMarker Marker;Marker.ID=It->CombatantID;Marker.Position=Position;VisibleMapMarkers.Add(Marker);}
    }
}
