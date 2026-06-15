// Copyright Jordan Chiquet 2025

#include "CableMovementComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"

UCableMovementComponent::UCableMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCableMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    StartLocation = Owner->GetActorLocation();
    EndLocation = StartLocation + FVector(0, 0, TravelHeight);

    if (CableSpline)
    {
        // Point 1 is the bottom of the cable, matching the actor's start position.
        CableStartLocation = CableSpline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
        CableEndLocation = CableStartLocation + FVector(0, 0, TravelHeight);
    }

    if (MovementCurve)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("UpdateMovement"));
        MovementTimeline.AddInterpFloat(MovementCurve, TimelineProgress);

        FOnTimelineEvent TimelineFinishedCallback;
        TimelineFinishedCallback.BindUFunction(this, FName("OnMovementTimelineFinished"));
        MovementTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
    }
}

void UCableMovementComponent::UpdateMovement(float Alpha)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    Owner->SetActorLocation(FMath::Lerp(StartLocation, EndLocation, Alpha));

    if (CableSpline)
    {
        // Keep the cable's bottom point locked to the actor as it moves.
        CableSpline->SetLocationAtSplinePoint(1,
            FMath::Lerp(CableStartLocation, CableEndLocation, Alpha),
            ESplineCoordinateSpace::World);
    }
}

void UCableMovementComponent::OnMovementTimelineFinished()
{
    OnMovementFinished.Broadcast();
}

void UCableMovementComponent::MoveUp()
{
    MovementTimeline.Play();
}

void UCableMovementComponent::MoveDown()
{
    MovementTimeline.Reverse();
}
