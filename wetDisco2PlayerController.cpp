// Copyright Epic Games, Inc. All Rights Reserved.

#include "wetDisco2PlayerController.h"

#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "wetDisco2Character.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/NavigationConfig.h"

#include "InteractiveActor.h"
#include "SMTLRadiusHelper.h"
#include "InteractableInterface.h"

#include "NavigationSystem.h"
#include "AIController.h"
#include "KeyboardInteractiveActorHandler.h"
#include "wetDisco2AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AwetDisco2PlayerController::AwetDisco2PlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	bMoveToMouseCursor = false;
	PendingInteractiveActor = nullptr;
}

void AwetDisco2PlayerController::CancelPendingInteractionFromBP(const FString& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("CancelPendingInteraction called from Blueprint. Reason: %s"), *Reason);
	CancelPendingInteraction();
}

void AwetDisco2PlayerController::BeginPlay()
{
	// Reclaim Tab/Shift+Tab from Slate navigation so they can be used as game input.
	TSharedRef<FNavigationConfig> Navigation = MakeShared<FNavigationConfig>();
	Navigation->bTabNavigation = false;
	FSlateApplication::Get().SetNavigationConfig(Navigation);

	OwnerPawn = GetPawn();
	Super::BeginPlay();
}

void AwetDisco2PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CheckPendingInteractionRange();
	UpdateInteractionCursor();
}

void AwetDisco2PlayerController::CameraUpdateEvent_Implementation()
{
	// Default C++ implementation is intentionally empty.
	// Blueprint overrides this to handle camera logic.
}

void AwetDisco2PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AwetDisco2PlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AwetDisco2PlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AwetDisco2PlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AwetDisco2PlayerController::OnSetDestinationReleased);

		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AwetDisco2PlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AwetDisco2PlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AwetDisco2PlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AwetDisco2PlayerController::OnTouchReleased);

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AwetDisco2PlayerController::OnInteractActionTriggered);
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AwetDisco2PlayerController::OnInteractActionCompleted);
		}

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AwetDisco2PlayerController::OnMoveTriggered);
	}
}

void AwetDisco2PlayerController::OnInputStarted()
{
	FHitResult Hit;
	if (GetHitResultUnderCursorForObjects(ClickableObjectTypes, true, Hit))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			// Actors with bUseMoveSphere=false (e.g. text bubbles) should trigger immediately
			// on click release without interrupting any active navigation. Flag this and return
			// early so OnInputStarted doesn't cancel movement.
			if (AInteractiveActor* Interactive = Cast<AInteractiveActor>(HitActor))
			{
				if (!Interactive->bUseMoveSphere)
				{
					bDestinationWasNonMoveSphereActor = true;
					return;
				}
			}
		}
	}

	if (!bDestinationWasNonMoveSphereActor)
	{
		StopMovement();
		CancelPendingInteraction();
	}
}

void AwetDisco2PlayerController::OnSetDestinationTriggered()
{
	if (bDestinationWasNonMoveSphereActor) return;

	bUsingKeyboardForMovement = false;
	FollowTime += GetWorld()->GetDeltaSeconds();

	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFingerForObjects(ETouchIndex::Touch1, ClickableObjectTypes, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursorForObjects(ClickableObjectTypes, true, Hit);
	}

	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}

	if (OwnerPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - OwnerPawn->GetActorLocation()).GetSafeNormal();
		OwnerPawn->AddMovementInput(WorldDirection, MovementInputScaleValue, false);
		CameraUpdateEvent();
	}
}

void AwetDisco2PlayerController::OnSetDestinationReleased()
{
	if (!bAcceptingMovement) return;

	if (bDestinationWasNonMoveSphereActor)
	{
		bDestinationWasNonMoveSphereActor = false;
		FollowTime = 0.f;
		return;
	}

	if (!PendingInteractiveActor && FollowTime <= ShortPressThreshold)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		CameraUpdateEvent();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

void AwetDisco2PlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	bUsingKeyboardForMovement = false;
	OnSetDestinationTriggered();
	CancelPendingInteraction();
}

void AwetDisco2PlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AwetDisco2PlayerController::OnInteractActionTriggered()
{
	// Interaction is intentionally resolved on Completed, not Triggered, to avoid
	// firing multiple times during a single button press.
}

void AwetDisco2PlayerController::OnInteractActionCompleted()
{
	if (!bAcceptingMovement) return;

	// KEYBOARD PATH: If the player is navigating via WASD, read the selected actor
	// from KeyboardInteractiveActorHandler and begin a pending interaction toward it.
	if (OwnerPawn && bUsingKeyboardForMovement)
	{
		UKeyboardInteractiveActorHandler* Handler = OwnerPawn->FindComponentByClass<UKeyboardInteractiveActorHandler>();
		if (Handler && Handler->SelectedInteractiveActor)
		{
			AActor* Target = Handler->SelectedInteractiveActor;

			if (Target->Implements<UInteractableInterface>())
			{
				USceneComponent* MoveToComp = IInteractableInterface::Execute_GetInteractionMoveToComponent(Target);
				bool bNeedsMovement = IInteractableInterface::Execute_RequiresMovementToInteract(Target);

				if (MoveToComp && bNeedsMovement)
				{
					BeginPendingInteraction(Target);
					USMTLRadiusHelper::SimpleMoveToLocationWithRadius(this, MoveToComp->GetComponentLocation(), MovementArrivalTolerance);
				}
				else
				{
					IInteractableInterface::Execute_ExecuteInteraction(Target, OwnerPawn);
				}
			}
		}
	}
	else
	{
		// CURSOR PATH: Trace under the cursor and either begin a pending interaction
		// (walk to actor, then trigger) or move to the hit location if nothing interactive was clicked.
		CancelPendingInteraction();
		bUsingKeyboardForMovement = false;

		FHitResult HitResult;
		bool bHit = GetHitResultUnderCursorForObjects(ClickableObjectTypes, true, HitResult);

		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && HitActor->Implements<UInteractableInterface>())
			{
				USceneComponent* MoveToComp = IInteractableInterface::Execute_GetInteractionMoveToComponent(HitActor);
				bool bNeedsMovement = IInteractableInterface::Execute_RequiresMovementToInteract(HitActor);

				UE_LOG(LogTemp, Warning, TEXT("HitActor: %s, MoveToComp: %s, bNeedsMovement: %d"),
					*HitActor->GetName(),
					MoveToComp ? TEXT("Valid") : TEXT("NULL"),
					bNeedsMovement);

				if (MoveToComp && bNeedsMovement)
				{
					BeginPendingInteraction(HitActor);
					USMTLRadiusHelper::SimpleMoveToLocationWithRadius(this, MoveToComp->GetComponentLocation(), MovementArrivalTolerance);
				}
				else
				{
					// Immediate interaction for actors that don't require navigation (e.g. bubble actors).
					IInteractableInterface::Execute_ExecuteInteraction(HitActor, OwnerPawn);
				}
			}
			else
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.Location);
			}
		}
	}
}

void AwetDisco2PlayerController::BeginPendingInteraction(AActor* TargetActor)
{
	if (TargetActor)
	{
		bInteractionJustStarted = true;
		PendingInteractiveActor = TargetActor;
		UE_LOG(LogTemp, Log, TEXT("Pending interaction set for: %s"), *TargetActor->GetName());
	}
}

void AwetDisco2PlayerController::CancelPendingInteraction()
{
	if (PendingInteractiveActor)
	{
		// Note: bInteractionJustStarted is intentionally NOT reset here. Resetting it
		// in CancelPendingInteraction broke the startup frame check in CheckPendingInteractionRange,
		// because Cancel gets called as cleanup whenever the player clicks a new destination.
		UE_LOG(LogTemp, Log, TEXT("Pending interaction cancelled for: %s"), *PendingInteractiveActor->GetName());
		PendingInteractiveActor = nullptr;
	}
}

void AwetDisco2PlayerController::ToggleAllHighlights(bool bEnable)
{
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->Implements<UInteractableInterface>())
		{
			if (IInteractableInterface::Execute_CanHighlight(Actor))
			{
				TArray<UPrimitiveComponent*> VisualComps;
				Actor->GetComponents<UPrimitiveComponent>(VisualComps);

				for (UPrimitiveComponent* Mesh : VisualComps)
				{
					if (Mesh)
					{
						Mesh->SetRenderCustomDepth(bEnable);

						if (bEnable)
						{
							// Stencil ID is fetched via interface so subclasses can define their own
							// outline color/style without this function needing to know their type.
							uint8 TargetStencil = IInteractableInterface::Execute_GetStencilID(Actor);
							Mesh->SetCustomDepthStencilValue(TargetStencil);
						}
					}
				}
			}
		}
	}
}

void AwetDisco2PlayerController::CheckPendingInteractionRange()
{
	if (!PendingInteractiveActor || !OwnerPawn) return;

	ACharacter* PlayerCharacter = Cast<ACharacter>(OwnerPawn);
	if (!PlayerCharacter) return;

	// On the first tick after an interaction is queued, the character may not have
	// started moving yet. Wait until movement has actually begun before checking arrival.
	if (bInteractionJustStarted)
	{
		if (PlayerCharacter->GetCharacterMovement()->Velocity.SizeSquared() > KINDA_SMALL_NUMBER ||
			(FindComponentByClass<UPathFollowingComponent>() && FindComponentByClass<UPathFollowingComponent>()->GetStatus() != EPathFollowingStatus::Idle))
		{
			bInteractionJustStarted = false;
		}
		return;
	}

	UPathFollowingComponent* PFollowComp = FindComponentByClass<UPathFollowingComponent>();
	bool bIsAIStillMoving = PFollowComp && (PFollowComp->GetStatus() != EPathFollowingStatus::Idle);

	// Still en route — keep waiting.
	if (PlayerCharacter->GetCharacterMovement()->Velocity.SizeSquared() > KINDA_SMALL_NUMBER || bIsAIStillMoving)
	{
		return;
	}

	USceneComponent* MoveToComp = IInteractableInterface::Execute_GetInteractionMoveToComponent(PendingInteractiveActor);
	if (MoveToComp)
	{
		float DistanceToSphereCenter = FVector::Dist(OwnerPawn->GetActorLocation(), MoveToComp->GetComponentLocation());

		if (DistanceToSphereCenter <= InteractArrivalTolerance + KINDA_SMALL_NUMBER)
		{
			// Arrived — face the target and fire the interaction.
			bInteractionJustStarted = false;
			TurnToInteractiveActor();
			IInteractableInterface::Execute_ExecuteInteraction(PendingInteractiveActor, OwnerPawn);
			CancelPendingInteraction();
			StopMovement();
		}
		else
		{
			// Stopped short of the target — cancel cleanly.
			UE_LOG(LogTemp, Warning, TEXT("Interaction Cancelled: Stopped too far (%.2f units, Tolerance: %.2f)"), DistanceToSphereCenter, InteractArrivalTolerance);
			CancelPendingInteraction();
		}
	}
}

void AwetDisco2PlayerController::TurnToInteractiveActor()
{
	if (OwnerPawn && PendingInteractiveActor)
	{
		FVector Direction = (PendingInteractiveActor->GetActorLocation() - OwnerPawn->GetActorLocation());
		Direction.Z = 0.0f;
		Direction.Normalize();

		if (!Direction.IsNearlyZero())
		{
			OwnerPawn->SetActorRotation(Direction.Rotation());
		}
	}
}

bool AwetDisco2PlayerController::IsUIAbsorbingInput() const
{
	// Hook for future UI input-blocking logic (e.g. suppress world clicks while a menu is open).
	return false;
}

void AwetDisco2PlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	bUsingKeyboardForMovement = true;
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (OwnerPawn != nullptr)
	{
		// If AI navigation is active, cancel it so WASD takes over cleanly.
		UPathFollowingComponent* PFollowComp = FindComponentByClass<UPathFollowingComponent>();
		if (PFollowComp && PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
		{
			StopMovement();
		}

		CancelPendingInteraction();

		// Orient movement relative to the camera so WASD always feels camera-relative.
		FRotator CameraRotation = FRotator::ZeroRotator;
		if (AwetDisco2Character* Disco2Character = Cast<AwetDisco2Character>(OwnerPawn))
		{
			if (Disco2Character->GetTopDownCameraComponent())
			{
				CameraRotation = Disco2Character->GetTopDownCameraComponent()->GetComponentRotation();
			}
		}
		else
		{
			CameraRotation = GetControlRotation();
		}

		const FRotator YawRotation(0, CameraRotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		OwnerPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		OwnerPawn->AddMovementInput(RightDirection, MovementVector.X);

		CameraUpdateEvent();
	}
}

void AwetDisco2PlayerController::UpdateInteractionCursor()
{
	if (!bChangeMouseIconAvailable)
	{
		CurrentMouseCursor = EMouseCursor::Default;
		return;
	}

	FHitResult HitResult;
	if (GetHitResultUnderCursorForObjects(ClickableObjectTypes, true, HitResult))
	{
		if (AInteractiveActor* IA = Cast<AInteractiveActor>(HitResult.GetActor()))
		{
			CurrentMouseCursor = IA->InteractionCursor;
			return;
		}
	}

	CurrentMouseCursor = EMouseCursor::Default;
}
