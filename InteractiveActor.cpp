// Copyright Jordan Chiquet 2025

#include "InteractiveActor.h"
#include "Components/SphereComponent.h"

AInteractiveActor::AInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // AInteractiveActor itself has no visual component as its root.
    // Subclasses define and set their own mesh components.
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    RootComponent = SceneRoot;

    // Sphere the player moves toward to perform the primary interaction.
    InteractMoveToSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractMoveToSphere"));
    InteractMoveToSphere->SetupAttachment(RootComponent);
    InteractMoveToSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractMoveToSphere->SetCollisionObjectType(ECC_GameTraceChannel2); // "OcclusionMask"
    InteractMoveToSphere->SetVisibility(true);
    InteractMoveToSphere->SetHiddenInGame(true);
    InteractMoveToSphere->SetSphereRadius(InteractMoveToSphereRadius);

    // Secondary move-to sphere used for alternate interaction scenarios (e.g. a second approach angle).
    InteractMoveToSphereBScenario = CreateDefaultSubobject<USphereComponent>(TEXT("InteractMoveToSphereBScenario"));
    InteractMoveToSphereBScenario->SetupAttachment(RootComponent);
    InteractMoveToSphereBScenario->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractMoveToSphereBScenario->SetCollisionObjectType(ECC_GameTraceChannel2); // "OcclusionMask"
    InteractMoveToSphereBScenario->SetVisibility(true);
    InteractMoveToSphereBScenario->SetHiddenInGame(true);
    InteractMoveToSphereBScenario->SetSphereRadius(InteractMoveToSphereBScenarioRadius);

    // Click-detection sphere for selecting this actor.
    // (Might swap collision settings on this one later — leaving as-is for now.)
    InteractClickSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractClickSphere"));
    InteractClickSphere->SetupAttachment(RootComponent);
    InteractClickSphere->SetSphereRadius(InteractClickSphereRadius);

    MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComponent"));
    RotatorComponent = CreateDefaultSubobject<URotatorComponent>(TEXT("RotatorComponent"));
}

void AInteractiveActor::BeginPlay()
{
    Super::BeginPlay();

    InteractMoveToSphere->SetRelativeLocation(MoveToSphereOffset);
    InteractMoveToSphereBScenario->SetRelativeLocation(MoveToSphereBScenarioOffset);
}

void AInteractiveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCalculateAnimSpeed)
    {
        return;
    }

    // Compute movement speed from frame-to-frame displacement so AnimBPs can drive
    // locomotion blends without duplicating this logic per-blueprint.
    const FVector CurrentLocation = GetActorLocation();
    const float DistanceMoved = FVector::Dist(CurrentLocation, PreviousLocation);

    if (DeltaTime > 0.f)
    {
        CurrentAnimSpeed = DistanceMoved / DeltaTime;
    }

    PreviousLocation = CurrentLocation;
}

// Default implementation for the BlueprintNativeEvent. Concrete C++ subclasses or
// Blueprint children are expected to override this with actual interaction behavior.
void AInteractiveActor::OnInteract_Implementation(APawn* InstigatorPawn)
{
    UE_LOG(LogTemp, Warning, TEXT("AInteractiveActor::OnInteract_Implementation called on abstract base. Implement in child Blueprint/C++! Actor: %s"), *GetName());
}

void AInteractiveActor::ToggleHighlightOn_Implementation(APawn* InstigatorPawn)
{
}

void AInteractiveActor::ToggleHighlightOff_Implementation(APawn* InstigatorPawn)
{
}

void AInteractiveActor::ExecuteInteraction_Implementation(APawn* InteractingEntity)
{
    OnInteract(InteractingEntity);
}

USceneComponent* AInteractiveActor::GetInteractionMoveToComponent_Implementation()
{
    return InteractMoveToSphere;
}

uint8 AInteractiveActor::GetStencilID_Implementation()
{
    return StencilID;
}
