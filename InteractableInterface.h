// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

class APawn;
class USceneComponent;

UINTERFACE(Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by any world actor the player can interact with.
 * Provides a consistent contract for interaction execution, move-to targeting,
 * highlight state, and stencil-based outline rendering.
 */
class WETDISCO2_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** Execute the primary interaction for this actor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void ExecuteInteraction(class APawn* InteractingEntity);

	/** Returns the scene component the player should navigate toward before interacting. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	class USceneComponent* GetInteractionMoveToComponent();

	/** Whether the player must walk to this actor before the interaction fires. Default: true. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool RequiresMovementToInteract();
	virtual bool RequiresMovementToInteract_Implementation() { return true; }

	/** Whether this actor responds to hover highlights. Default: true. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanHighlight();
	virtual bool CanHighlight_Implementation() { return true; }

	/** Returns the custom depth stencil ID used for outline rendering. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	uint8 GetStencilID();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void ToggleHighlightOn(class APawn* InPawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void ToggleHighlightOff(class APawn* InPawn);
};
