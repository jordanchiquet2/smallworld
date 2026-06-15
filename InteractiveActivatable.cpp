// Copyright Jordan Chiquet 2025

#include "InteractiveActivatable.h"

AInteractiveActivatable::AInteractiveActivatable()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(GetRootComponent());
}
