// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "InteractiveContainer.generated.h"

/** A container actor (chest, cupboard, etc.) that holds a set of item IDs. */
UCLASS()
class WETDISCO2_API AInteractiveContainer : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AInteractiveContainer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	TArray<FName> ContainedItemIDs;
};
