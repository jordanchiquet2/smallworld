// Copyright Jordan Chiquet 2025

#include "SMTLRadiusHelper.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Controller.h"
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "SMTLRadiusHelper"

void USMTLRadiusHelper::SimpleMoveToLocationWithRadius(AController* Controller, const FVector& GoalLocation, float AcceptanceRadius)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller ? Controller->GetWorld() : nullptr);
	if (NavSys == nullptr || Controller == nullptr || Controller->GetPawn() == nullptr)
	{
		UE_LOG(LogNavigation, Error, TEXT("SMTL ABORT: Invalid Controller, Pawn, or NavigationSystem."));
		return;
	}

	// Replicate the engine's private logic for resolving the PathFollowingComponent,
	// which differs between AIControllers (already have one) and PlayerControllers (may not).
	UPathFollowingComponent* PFollowComp = nullptr;
	AAIController* AsAIController = Cast<AAIController>(Controller);
	if (AsAIController)
	{
		PFollowComp = AsAIController->GetPathFollowingComponent();
	}
	else
	{
		PFollowComp = Controller->FindComponentByClass<UPathFollowingComponent>();
		if (PFollowComp == nullptr)
		{
			UE_LOG(LogNavigation, Warning, TEXT("SMTL: No PathFollowingComponent on PlayerController — creating one."));
			PFollowComp = NewObject<UPathFollowingComponent>(Controller, TEXT("PathFollowingComponent"));
			if (PFollowComp)
			{
				PFollowComp->RegisterComponent();
				PFollowComp->Initialize();
			}
		}
	}

	if (PFollowComp == nullptr)
	{
		UE_LOG(LogNavigation, Error, TEXT("SMTL ABORT: Failed to get or create a PathFollowingComponent."));
		return;
	}

	if (!PFollowComp->IsPathFollowingAllowed())
	{
		UE_LOG(LogNavigation, Warning, TEXT("SMTL ABORT: Movement is not allowed by PathFollowingComponent."));
		return;
	}

	const bool bAlreadyAtGoal = PFollowComp->HasReached(GoalLocation, EPathFollowingReachMode::ExactLocation, AcceptanceRadius);

	if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest,
			FAIRequestID::AnyRequest, bAlreadyAtGoal ? EPathFollowingVelocityMode::Reset : EPathFollowingVelocityMode::Keep);
	}

	if (bAlreadyAtGoal)
	{
		PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
	}
	else
	{
		const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), Controller->GetNavAgentLocation());
		if (NavData)
		{
			FPathFindingQuery Query(Controller, *NavData, Controller->GetNavAgentLocation(), GoalLocation);
			FPathFindingResult Result = NavSys->FindPathSync(Query);

			if (Result.IsSuccessful())
			{
				FAIMoveRequest MoveReq(GoalLocation);
				MoveReq.SetAcceptanceRadius(AcceptanceRadius);
				MoveReq.SetUsePathfinding(true);
				PFollowComp->RequestMove(MoveReq, Result.Path);
			}
			else
			{
				UE_LOG(LogNavigation, Error, TEXT("SMTL: Pathfinding failed. Reason: %s"), *UEnum::GetValueAsString(Result.Result));
				if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
				{
					PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
				}
			}
		}
		else
		{
			UE_LOG(LogNavigation, Error, TEXT("SMTL ABORT: Failed to get NavData for pawn."));
		}
	}
}

#undef LOCTEXT_NAMESPACE
