// Copyright Jordan Chiquet 2025

#include "RotatorComponent.h"
#include "GameFramework/Actor.h"

URotatorComponent::URotatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URotatorComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	StartRotation = Owner->GetActorRotation();

	if (RotationCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("UpdateRotation"));
		RotationTimeline.AddInterpFloat(RotationCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("OnTimelineFinished"));
		RotationTimeline.SetTimelineFinishedFunc(FinishedFunction);

		if (RotationDuration > 0.0f)
		{
			RotationTimeline.SetPlayRate(1.0f / RotationDuration);
		}
	}

	if (bStartOnBeginPlay)
	{
		StartRotationForward();
	}
}

void URotatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RotationTimeline.TickTimeline(DeltaTime);
}

void URotatorComponent::UpdateRotation(float Alpha)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorRotation(FMath::Lerp(StartRotation, EndRotation, Alpha));
	}
}

void URotatorComponent::OnTimelineFinished()
{
	if (RotationTimeline.GetPlaybackPosition() == 0.0f) // Finished in reverse
	{
		if (bShouldLoop)
		{
			UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: Finished Backward. Looping Forward."), *GetOwner()->GetName());
			StartRotationForward();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: ROTATION FINISHED (At Start Rotation)."), *GetOwner()->GetName());
			OnRotationFinished.Broadcast();
		}
	}
	else // Finished forward
	{
		if (bShouldLoop)
		{
			UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: Finished Forward. Looping Backward."), *GetOwner()->GetName());
			StartRotationBackward();
		}
		else if (bShouldReturnToStart)
		{
			UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: Finished Forward. Returning to Start."), *GetOwner()->GetName());
			StartRotationBackward();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: ROTATION FINISHED (At End Rotation)."), *GetOwner()->GetName());
			OnRotationFinished.Broadcast();
		}
	}
}

void URotatorComponent::StartRotationForward()
{
	UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: STARTING ROTATION FORWARD. From: %s | To: %s | Duration: %.2fs"),
		*GetOwner()->GetName(),
		*StartRotation.ToString(),
		*EndRotation.ToString(),
		RotationDuration);

	RotationTimeline.Play();
}

void URotatorComponent::StartRotationBackward()
{
	UE_LOG(LogTemp, Log, TEXT("[RotatorComponent] %s: STARTING ROTATION BACKWARD. Returning to: %s | Duration: %.2fs"),
		*GetOwner()->GetName(),
		*StartRotation.ToString(),
		RotationDuration);

	RotationTimeline.Reverse();
}
