// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "Components/TimelineComponent.h"
#include "CableMovementComponent.generated.h"

/** Broadcasts when the cable movement sequence completes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableMovementFinished);

/**
 * Moves an actor vertically along a cable, simultaneously animating the bottom
 * spline point to simulate the cable shortening or lengthening. Designed for
 * pulley/lift scenarios where a visible cable needs to stay attached to the payload.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WETDISCO2_API UCableMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public: 
    UCableMovementComponent();

protected:
    virtual void BeginPlay() override;

    /** The spline representing the cable. Expects exactly two points; point 1 is the bottom (payload end). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Movement|Setup", meta = (UseComponentPicker = "true"))
    USplineComponent* CableSpline;

    /** Duration of the full up or down movement in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Movement|Settings")
    float MovementDuration = 5.0f;

    /** Vertical distance the actor travels, relative to its starting position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Movement|Settings")
    float TravelHeight = 500.0f;

    /** Optional easing curve for the movement. */
    UPROPERTY(EditAnywhere, Category = "Cable Movement|Settings")
    UCurveFloat* MovementCurve;

public: 
    UFUNCTION(BlueprintCallable, Category = "Cable Movement")
    void MoveUp();

    UFUNCTION(BlueprintCallable, Category = "Cable Movement")
    void MoveDown();

    UPROPERTY(BlueprintAssignable, Category = "Cable Movement")
    FOnCableMovementFinished OnMovementFinished;

private:
    FTimeline MovementTimeline;

    FVector StartLocation;
    FVector EndLocation;
    FVector CableStartLocation;
    FVector CableEndLocation;

    UFUNCTION()
    void UpdateMovement(float Alpha);

    UFUNCTION()
    void OnMovementTimelineFinished();
};
