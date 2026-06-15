// Copyright Jordan Chiquet 2025

#include "InteractiveContainer.h"

AInteractiveContainer::AInteractiveContainer()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(GetRootComponent());
}
