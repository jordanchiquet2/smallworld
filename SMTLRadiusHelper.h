// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SMTLRadiusHelper.generated.h"

/**
 * Utility library that extends UAIBlueprintHelperLibrary::SimpleMoveToLocation with
 * a configurable acceptance radius. The engine's built-in version doesn't expose this
 * parameter, so this replicates its internal logic with radius support added.
 */
UCLASS()
class WETDISCO2_API USMTLRadiusHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Moves a pawn to the specified goal location, stopping within AcceptanceRadius.
	 * Works for both AIControllers and PlayerControllers — creates a PathFollowingComponent
	 * on the fly if the PlayerController doesn't already have one.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static void SimpleMoveToLocationWithRadius(AController* Controller, const FVector& Goal, float AcceptanceRadius = 5.f);
};
