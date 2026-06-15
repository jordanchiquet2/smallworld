// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "MoverComponent.h"
#include "RotatorComponent.h"
#include "InteractableInterface.h"

#include "InteractiveActor.generated.h"

class USphereComponent;
class APawn;
class USceneComponent;

/**
 * Abstract base class for any actor the player can target and interact with.
 * Provides click detection, move-to targeting (including a secondary "B scenario"
 * approach point), highlight state, gameplay-tag-gated usability, and optional
 * timeline-driven motion via MoverComponent/RotatorComponent.
 */
UCLASS(Abstract)
class WETDISCO2_API AInteractiveActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractiveActor();

	/** The cursor to display when this actor is hovered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TEnumAsByte<EMouseCursor::Type> InteractionCursor = EMouseCursor::Default;

	/**
	 * Primary interaction entry point. BlueprintNativeEvent so it can be implemented
	 * in C++ (via _Implementation) or overridden entirely in Blueprint.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteract(APawn* InstigatorPawn);

	/** Other actors linked to this one for coordinated movement (e.g. a switch that moves doors). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
	TArray<AActor*> LinkedMovementActors;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void ToggleHighlightOn(APawn* InstigatorPawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void ToggleHighlightOff(APawn* InstigatorPawn);

	/** Optional component for moving the actor via a timeline. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Motion")
	UMoverComponent* MoverComponent;

	/** Optional component for rotating the actor via a timeline. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Motion")
	URotatorComponent* RotatorComponent;

	/** If false, the player moves directly to InteractClickSphere and interacts immediately without pathing to a move-to sphere.
	 *  (Text-bubble-style actors probably shouldn't require movement at all — added this flag for that case.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bUseMoveSphere = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	uint8 StencilID = 1;

	/** Sphere the player navigates to before the primary interaction triggers. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractMoveToSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractMoveToSphereRadius = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FVector MoveToSphereOffset;

	/** Alternate move-to sphere/transform, used for secondary interaction scenarios. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractMoveToSphereBScenario;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractMoveToSphereBScenarioRadius = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FVector MoveToSphereBScenarioOffset;

	/** Sphere used to detect clicks/hover for this actor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractClickSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractClickSphereRadius = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanBeUsed = true;

	/** Tags the instigating pawn must have for this actor to be usable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags")
	FGameplayTagContainer TagsRequiredToUse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion|Default Settings")
	FVector DefaultMoveEndLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion|Default Settings")
	FRotator DefaultRotateEndRotation;

	/** Applied to both MoverComponent and RotatorComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion|Default Settings", DisplayName = "Default Motion Duration")
	float DefaultMotionDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion|Default Settings")
	bool bShouldLoopDefaultMotion = false;

	/** Enable for actors whose AnimBP needs CurrentAnimSpeed (e.g. characters); leave off for static actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bCalculateAnimSpeed = false;

	/** Speed calculated from frame-to-frame movement, for AnimBPs to read. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float CurrentAnimSpeed = 0.0f;

protected:
	/** Lightweight root component for this abstract base; subclasses attach their own meshes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	virtual void BeginPlay() override;

	FVector PreviousLocation;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void ExecuteInteraction_Implementation(class APawn* InteractingEntity) override;
	virtual class USceneComponent* GetInteractionMoveToComponent_Implementation() override;
	virtual uint8 GetStencilID_Implementation() override;
};
