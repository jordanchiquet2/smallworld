// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "wetDisco2AIController.generated.h"

/**
 * Base AIController for SmallWorld NPCs. Behavior logic is implemented in Blueprint
 * child classes; this C++ base exists to allow shared C++ functionality to be added
 * without touching Blueprint graphs.
 */
UCLASS()
class WETDISCO2_API AwetDisco2AIController : public AAIController
{
	GENERATED_BODY()

public:
	AwetDisco2AIController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
