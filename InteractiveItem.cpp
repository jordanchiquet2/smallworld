// Copyright Jordan Chiquet 2025

#include "InteractiveItem.h"

AInteractiveItem::AInteractiveItem()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(GetRootComponent());

	ItemID = TEXT("DefaultItem");
}

void AInteractiveItem::OnUseItem_Implementation(APawn* InstigatorPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("AInteractiveItem::OnUseItem_Implementation called on base class. Implement in child Blueprint/C++! Actor: %s"), *GetName());
}
