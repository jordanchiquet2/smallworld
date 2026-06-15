// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractiveActor.h"
#include "KeyboardInteractiveActorHandler.generated.h"

/**
 * Component attached to the player pawn that manages interactive actor selection
 * during WASD (keyboard) movement. Each tick, it performs a forward-facing cone
 * overlap to find the most-aligned interactive actor and selects it. The player
 * controller reads SelectedInteractiveActor when the interact key is pressed.
 *
 * Also supports manual cycling (tab-style) through nearby actors via CycleInteraction,
 * which switches to a broader radius sort until the player starts moving again.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WETDISCO2_API UKeyboardInteractiveActorHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeyboardInteractiveActorHandler();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Returns true when the player controller is in keyboard movement mode. */
	bool bIsDetectionEnabled();

	/** Cycles selection to the next (1) or previous (-1) interactive actor within SelectionRadius. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CycleInteraction(int32 Direction);

	/** Radius for the broad overlap used during manual cycling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	float SelectionRadius = 600.0f;

	/** Half-angle of the forward-facing detection cone in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	float InteractiveConeAngle = 90.0f;

	/** Range of the detection cone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	float InteractiveConeRange = 300.0f;

	/** Draw the detection cone in the viewport (editor/debug use). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
	bool bShowDebugVisualizationCone = false;

	/** Draw the selection radius sphere in the viewport (editor/debug use). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
	bool bShowDebugVisualizationSphere = false;

	/** The actor currently prioritized for interaction. Read by the player controller on interact input. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|State")
	AActor* SelectedInteractiveActor;

	/** All interactive actors currently within the detection cone. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|State")
	TArray<AActor*> FoundInteractiveActors;

private:
	void PerformConeDetection();
	void DrawInteractionDebugCone();
	void DrawInteractionDebugSphere();

	/** Sorts FoundInteractiveActors by alignment with the owner's forward vector. */
	void SortFoundActors();

	/** Queries the NavMesh to confirm a target actor is actually reachable before selecting it. */
	bool IsActorReachable(AActor* TargetActor);

	int32 CurrentCycleIndex = 0;

	/** True while the player is manually cycling through actors rather than using cone auto-select. */
	bool bManualCycleActive = false;
};
