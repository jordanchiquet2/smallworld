// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Templates/SubclassOf.h"

#include "wetDisco2PlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class AInteractiveActor;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * Handles all player input, navigation, and interaction dispatch for SmallWorld.
 *
 * Supports two input modes that can switch dynamically at runtime:
 *   - Click-to-move: the player clicks a world location or interactive actor;
 *     the controller navigates the pawn via AI pathfinding and fires the interaction
 *     on arrival (pending interaction system).
 *   - WASD: direct movement input, camera-relative. The KeyboardInteractiveActorHandler
 *     component handles actor selection in this mode.
 *
 * Interaction flow (both modes): click/press interact → BeginPendingInteraction →
 * pawn navigates to actor's InteractMoveToSphere → CheckPendingInteractionRange fires
 * on arrival → TurnToInteractiveActor → ExecuteInteraction via IInteractableInterface.
 */
UCLASS()
class AwetDisco2PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AwetDisco2PlayerController();

	/** Exposed to Blueprint so dialogue/cutscene logic can cancel a pending interaction cleanly. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelPendingInteractionFromBP(const FString& Reason);

	/** If false, the cursor icon is locked to Default regardless of what's under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	bool bChangeMouseIconAvailable = true;

	/** Object types that register as valid click/hover targets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Click Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> ClickableObjectTypes;

	/** Max duration of a mouse press that counts as a "short press" (navigates to location rather than dragging). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float ShortPressThreshold;

	/** While false, all movement input (click and WASD) is suppressed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bAcceptingMovement = true;

	/** Niagara effect spawned at the click destination. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UNiagaraSystem* FXCursor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SetDestinationClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SetDestinationTouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** BlueprintNativeEvent — Blueprint overrides this to drive camera behavior on movement. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void CameraUpdateEvent();

	/** Enables or disables custom depth (outline) rendering on all highlightable interactive actors. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ToggleAllHighlights(bool bEnable);

	/** Exposed so Blueprint can read which input mode is active (e.g. to show controller hints). */
	bool bUsingKeyboardForMovement = false;

	/** Exposed to Blueprint to fire debug skill-setting commands via Articy. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug|Console")
	void K2_OnSetSkillCommand(const FString& SkillName, int32 Value);

protected:
	/** True if the pawn should navigate toward the mouse cursor each tick. */
	uint32 bMoveToMouseCursor : 1;

	/** Updates CurrentMouseCursor based on what interactive actor (if any) is under the cursor. */
	void UpdateInteractionCursor();

	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();

	void OnMoveTriggered(const FInputActionValue& Value);

	void OnInteractActionTriggered();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void OnInteractActionCompleted();

	/** Stores a target actor and begins navigating toward it. Interaction fires on arrival. */
	void BeginPendingInteraction(AActor* TargetActor);

	/** Clears the pending interaction without firing it. */
	void CancelPendingInteraction();

	/** Called each tick while a pending interaction is active. Fires the interaction when the pawn arrives. */
	void CheckPendingInteractionRange();

	/** Distance tolerance for considering the pawn "arrived" at a pending interaction target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractArrivalTolerance = 10;

	/** Distance tolerance for general navigation arrival. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float MovementArrivalTolerance = 10;

	/** Hook for future UI input-blocking logic (e.g. suppress world clicks while a menu is open). */
	bool IsUIAbsorbingInput() const;

private:
	FVector CachedDestination;
	bool bIsTouch;
	float FollowTime;

	/** Scale value passed to AddMovementInput during click-drag movement. */
	float MovementInputScaleValue = 1;

	/** The actor the pawn is currently navigating toward to interact with. */
	UPROPERTY()
	AActor* PendingInteractiveActor;

	APawn* OwnerPawn;

	/** Rotates the pawn to face PendingInteractiveActor just before firing the interaction. */
	void TurnToInteractiveActor();

	/** True for the first tick(s) after an interaction is queued, before the pawn has started moving. */
	bool bInteractionJustStarted = false;

	/** True if the click that started the current input was on a non-move-sphere actor (e.g. a text bubble). */
	bool bDestinationWasNonMoveSphereActor;
};
