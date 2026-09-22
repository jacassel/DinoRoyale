#include "DinoPlayerState.h"
#include "Net/UnrealNetwork.h"
void ADinoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(ADinoPlayerState,CombatantID);DOREPLIFETIME(ADinoPlayerState,SelectedSpecies);}
