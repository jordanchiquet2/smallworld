// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "InteractiveActivatable.generated.h"

/** A triggerable actor (lever, button, door, etc.). Interaction logic is implemented in Blueprint. */
UCLASS()
class WETDISCO2_API AInteractiveActivatable : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AInteractiveActivatable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;
};
