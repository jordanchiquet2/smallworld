// Copyright Jordan Chiquet 2025

#include "MoverComponent.h"
#include "GameFramework/Actor.h"

UMoverComponent::UMoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	StartLocation = Owner->GetActorLocation();

	if (MovementCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("UpdateMovement"));
		MovementTimeline.AddInterpFloat(MovementCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("OnTimelineFinished"));
		MovementTimeline.SetTimelineFinishedFunc(FinishedFunction);

		// Scale the play rate so the normalized 0-1 curve spans MoveDuration seconds.
		if (MoveDuration > 0.0f)
		{
			MovementTimeline.SetPlayRate(1.0f / MoveDuration);
		}
	}

	if (bStartOnBeginPlay)
	{
		StartMovementForward();
	}
}

void UMoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	MovementTimeline.TickTimeline(DeltaTime);
}

void UMoverComponent::UpdateMovement(float Alpha)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorLocation(FMath::Lerp(StartLocation, EndLocation, Alpha));
	}
}

void UMoverComponent::OnTimelineFinished()
{
	if (MovementTimeline.GetPlaybackPosition() == 0.0f) // Finished in reverse
	{
		if (bShouldLoop)
		{
			UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: Finished Backward. Looping Forward."), *GetOwner()->GetName());
			StartMovementForward();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: MOVEMENT FINISHED (At Start Location)."), *GetOwner()->GetName());
			OnMovementFinished.Broadcast();
		}
	}
	else // Finished forward
	{
		if (bShouldLoop)
		{
			UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: Finished Forward. Looping Backward."), *GetOwner()->GetName());
			StartMoveBackward();
		}
		else if (bShouldReturnToStart)
		{
			UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: Finished Forward. Returning to Start."), *GetOwner()->GetName());
			StartMoveBackward();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: MOVEMENT FINISHED (At End Location)."), *GetOwner()->GetName());
			OnMovementFinished.Broadcast();
		}
	}
}

void UMoverComponent::StartMovementForward()
{
	// Always snapshot current position as the start so mid-motion reversals work correctly.
	if (AActor* Owner = GetOwner())
	{
		StartLocation = Owner->GetActorLocation();
	}

	UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: STARTING MOVE FORWARD. From: %s | To: %s | Duration: %.2fs"),
		*GetOwner()->GetName(),
		*StartLocation.ToString(),
		*EndLocation.ToString(),
		MoveDuration);

	if (MoveDuration > 0.0f)
	{
		MovementTimeline.SetPlayRate(1.0f / MoveDuration);
	}

	MovementTimeline.PlayFromStart();
}

void UMoverComponent::StartMoveBackward()
{
	UE_LOG(LogTemp, Log, TEXT("[MoverComponent] %s: STARTING MOVE BACKWARD. Returning to: %s | Duration: %.2fs"),
		*GetOwner()->GetName(),
		*StartLocation.ToString(),
		MoveDuration);

	MovementTimeline.Reverse();
}
