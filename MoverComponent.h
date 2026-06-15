// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "MoverComponent.generated.h"

/** Broadcasts when a non-looping movement sequence completes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoverFinished);

/**
 * Timeline-driven movement component. Drop onto any actor and configure entirely
 * from the editor — target location, duration, easing curve, loop/ping-pong behavior.
 * Offset mode (bOffsetting) allows specifying displacement rather than a world destination,
 * which is more convenient for simple push/pull scenarios in Blueprint.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WETDISCO2_API UMoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMoverComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Target world-space location for the actor to move to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	FVector EndLocation;

	/** Used with bOffsetting — displacement added to the actor's current location rather than a world destination. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	FVector OffsetFromStart;

	/** Time in seconds for a one-way movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	float MoveDuration = 3.0f;

	/** Float curve for easing. */
	UPROPERTY(EditAnywhere, Category = "Mover|Settings")
	UCurveFloat* MovementCurve;

	/** If true, movement starts automatically on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	bool bStartOnBeginPlay = false;

	/** If true, EndLocation is treated as an offset from the actor's current position rather than a world destination. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	bool bOffsetting = false;

	/** If true, the actor returns to its start location after reaching EndLocation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	bool bShouldReturnToStart = false;

	/** If true, the actor ping-pongs between start and end indefinitely. Overrides bShouldReturnToStart. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mover|Settings")
	bool bShouldLoop = false;

	UFUNCTION(BlueprintCallable, Category = "Mover")
	void StartMovementForward();

	UFUNCTION(BlueprintCallable, Category = "Mover")
	void StartMoveBackward();

	/** Broadcasts when a non-looping movement sequence completes. */
	UPROPERTY(BlueprintAssignable, Category = "Mover")
	FOnMoverFinished OnMovementFinished;

private:
	FTimeline MovementTimeline;
	FVector StartLocation;

	UFUNCTION()
	void UpdateMovement(float Alpha);

	UFUNCTION()
	void OnTimelineFinished();
};
