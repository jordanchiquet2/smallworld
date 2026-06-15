// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "InteractiveItem.generated.h"

/** A pickup-able / usable item actor (e.g. inventory items found in the world). */
UCLASS()
class WETDISCO2_API AInteractiveItem : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AInteractiveItem();

	/** Called when the player uses this item (e.g. from an inventory). */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnUseItem(APawn* InstigatorPawn);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	/** Unique ID for inventory system lookup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemID;
};
