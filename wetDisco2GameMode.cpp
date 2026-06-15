// Copyright Epic Games, Inc. All Rights Reserved.

#include "wetDisco2GameMode.h"
#include "wetDisco2PlayerController.h"
#include "wetDisco2Character.h"
#include "ArticyGlobalVariables.h"
#include "UObject/ConstructorHelpers.h"

AwetDisco2GameMode::AwetDisco2GameMode()
{
	// Default pawn and controller classes are set in Blueprint; only the C++ controller
	// class is bound here so the PlayerController is always the correct type at startup.
	PlayerControllerClass = AwetDisco2PlayerController::StaticClass();

	// Note: attempted to reset Articy global variables here via UArticyGlobalVariables::UnloadGlobalVariables()
	// during level transitions, but the approach was unreliable. Global variable persistence
	// across levels is now managed manually on the Game Instance instead.
}
