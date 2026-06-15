// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "RotatorComponent.generated.h"

/** Broadcasts when a non-looping rotation sequence completes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotatorFinished);

/**
 * Timeline-driven rotation component. Mirrors the interface of MoverComponent
 * but operates on rotation rather than translation. Fully configurable from
 * the editor — target rotation, duration, easing curve, loop/ping-pong.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WETDISCO2_API URotatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URotatorComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Target world-space rotation for the actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator|Settings")
	FRotator EndRotation;

	/** Time in seconds for a one-way rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator|Settings")
	float RotationDuration = 3.0f;

	/** Float curve for easing. */
	UPROPERTY(EditAnywhere, Category = "Rotator|Settings")
	UCurveFloat* RotationCurve;

	/** If true, rotation starts automatically on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator|Settings")
	bool bStartOnBeginPlay = false;

	/** If true, the actor returns to its start rotation after reaching EndRotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator|Settings")
	bool bShouldReturnToStart = false;

	/** If true, the actor ping-pongs between start and end indefinitely. Overrides bShouldReturnToStart. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator|Settings")
	bool bShouldLoop = false;

	UFUNCTION(BlueprintCallable, Category = "Rotator")
	void StartRotationForward();

	UFUNCTION(BlueprintCallable, Category = "Rotator")
	void StartRotationBackward();

	/** Broadcasts when a non-looping rotation sequence completes. */
	UPROPERTY(BlueprintAssignable, Category = "Rotator")
	FOnRotatorFinished OnRotationFinished;

private:
	FTimeline RotationTimeline;
	FRotator StartRotation;

	UFUNCTION()
	void UpdateRotation(float Alpha);

	UFUNCTION()
	void OnTimelineFinished();
};
