// Copyright Jordan Chiquet 2025

#include "InteractiveDialogueActor.h"

AInteractiveDialogueActor::AInteractiveDialogueActor()
{
}

// C++ provides only a stub here; the actual dialogue trigger (Articy) is wired up in Blueprint.
void AInteractiveDialogueActor::OnInteract_Implementation(APawn* InstigatorPawn)
{
	UE_LOG(LogTemp, Log, TEXT("SHOULD DO DIALOGUE with %s by %s!"),
		*GetName(), InstigatorPawn ? *InstigatorPawn->GetName() : TEXT("Unknown Instigator"));
}
