// Copyright Jordan Chiquet 2025

#include "KeyboardInteractiveActorHandler.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavigationData.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "wetDisco2/wetDisco2PlayerController.h"
#include "InteractableInterface.h"
#include "InteractiveActor.h"

UKeyboardInteractiveActorHandler::UKeyboardInteractiveActorHandler()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UKeyboardInteractiveActorHandler::BeginPlay()
{
    Super::BeginPlay();
}

void UKeyboardInteractiveActorHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Snapshot selection before updating so we can detect changes for highlight transitions.
    AActor* OldActor = SelectedInteractiveActor;

    // If the player starts moving while manually cycling, revert to automatic cone detection.
    if (bManualCycleActive && Owner->GetVelocity().Size() > 10.0f)
    {
        bManualCycleActive = false;
        UE_LOG(LogTemp, Warning, TEXT("Wetdisco Interaction: Movement detected, resetting to Cone mode."));
    }

    if (bIsDetectionEnabled())
    {
        PerformConeDetection();
    }
    else
    {
        SelectedInteractiveActor = nullptr;
        FoundInteractiveActors.Empty();
    }

    // When selection changes, toggle highlight off the old actor and on the new one.
    if (OldActor != SelectedInteractiveActor)
    {
        if (OldActor && OldActor->Implements<UInteractableInterface>())
        {
            if (IInteractableInterface::Execute_CanHighlight(OldActor))
            {
                TArray<UPrimitiveComponent*> VisualComps;
                OldActor->GetComponents<UPrimitiveComponent>(VisualComps);
                for (UPrimitiveComponent* Mesh : VisualComps)
                {
                    Mesh->SetRenderCustomDepth(false);
                }
                IInteractableInterface::Execute_ToggleHighlightOff(OldActor, Cast<APawn>(Owner));
            }
        }

        if (SelectedInteractiveActor && SelectedInteractiveActor->Implements<UInteractableInterface>())
        {
            if (IInteractableInterface::Execute_CanHighlight(SelectedInteractiveActor))
            {
                uint8 StencilToUse = IInteractableInterface::Execute_GetStencilID(SelectedInteractiveActor);

                TArray<UPrimitiveComponent*> VisualComps;
                SelectedInteractiveActor->GetComponents<UPrimitiveComponent>(VisualComps);
                for (UPrimitiveComponent* Mesh : VisualComps)
                {
                    Mesh->SetRenderCustomDepth(true);
                    Mesh->SetCustomDepthStencilValue(StencilToUse);
                }
                IInteractableInterface::Execute_ToggleHighlightOn(SelectedInteractiveActor, Cast<APawn>(Owner));
            }
        }
    }

    if (bShowDebugVisualizationCone)
    {
        DrawInteractionDebugCone();
    }

    if (bShowDebugVisualizationSphere)
    {
        DrawInteractionDebugSphere();
    }
}

bool UKeyboardInteractiveActorHandler::bIsDetectionEnabled()
{
    AwetDisco2PlayerController* PC = Cast<AwetDisco2PlayerController>(GetOwner()->GetInstigatorController());
    return (PC != nullptr) ? PC->bUsingKeyboardForMovement : false;
}

void UKeyboardInteractiveActorHandler::PerformConeDetection()
{
    AActor* Owner = GetOwner();
    if (!Owner || !bIsDetectionEnabled()) return;

    FVector StartLoc = Owner->GetActorLocation();
    FVector ForwardDir = Owner->GetActorForwardVector();
    ForwardDir.Z = 0.0f;
    ForwardDir = ForwardDir.GetSafeNormal();

    if (!bManualCycleActive)
    {
        FoundInteractiveActors.Empty();
        SelectedInteractiveActor = nullptr;

        TArray<FOverlapResult> Overlaps;
        FCollisionShape ConeSphere = FCollisionShape::MakeSphere(InteractiveConeRange);
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Owner);

        if (GetWorld()->OverlapMultiByChannel(Overlaps, StartLoc, FQuat::Identity, ECC_Visibility, ConeSphere, Params))
        {
            float BestDot = -1.0f;
            float HalfAngleRad = FMath::DegreesToRadians(InteractiveConeAngle * 0.5f);

            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* PotentialActor = Overlap.GetActor();
                if (!PotentialActor || !PotentialActor->Implements<UInteractableInterface>()) continue;
                if (PotentialActor->IsHidden()) continue;

                // Use the actor's move-to component location for angle calculation if available.
                USceneComponent* MoveComp = IInteractableInterface::Execute_GetInteractionMoveToComponent(PotentialActor);
                FVector TargetLoc = MoveComp ? MoveComp->GetComponentLocation() : PotentialActor->GetActorLocation();

                FVector DirToActor = (PotentialActor->GetActorLocation() - StartLoc);
                DirToActor.Z = 0.0f;
                DirToActor = DirToActor.GetSafeNormal();

                float CurrentDot = FVector::DotProduct(ForwardDir, DirToActor);

                if (CurrentDot >= FMath::Cos(HalfAngleRad))
                {
                    FoundInteractiveActors.Add(PotentialActor);
                    if (CurrentDot > BestDot)
                    {
                        BestDot = CurrentDot;
                        SelectedInteractiveActor = PotentialActor;
                    }
                }
            }
        }
    }
    else
    {
        // In manual cycle mode — just validate that the current selection is still usable.
        if (!SelectedInteractiveActor || SelectedInteractiveActor->IsHidden() || !IsActorReachable(SelectedInteractiveActor))
        {
            bManualCycleActive = false;
        }
    }
}

void UKeyboardInteractiveActorHandler::CycleInteraction(int32 Direction)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape BroadSphere = FCollisionShape::MakeSphere(SelectionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    TArray<AActor*> NearbyActors;
    if (GetWorld()->OverlapMultiByChannel(Overlaps, Owner->GetActorLocation(), FQuat::Identity, ECC_Visibility, BroadSphere, Params))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* PotentialActor = Overlap.GetActor();
            if (PotentialActor && PotentialActor->Implements<UInteractableInterface>() && !PotentialActor->IsHidden() && IsActorReachable(PotentialActor))
            {
                NearbyActors.AddUnique(PotentialActor);
            }
        }
    }

    if (NearbyActors.Num() == 0) return;

    // Sort by distance so cycling moves through actors in a predictable spatial order.
    NearbyActors.Sort([Owner](const AActor& A, const AActor& B) {
        return FVector::DistSquared(Owner->GetActorLocation(), A.GetActorLocation()) < FVector::DistSquared(Owner->GetActorLocation(), B.GetActorLocation());
    });

    if (!bManualCycleActive)
    {
        bManualCycleActive = true;
        //comment testing perforce
        FoundInteractiveActors = NearbyActors;
        CurrentCycleIndex = FoundInteractiveActors.Find(SelectedInteractiveActor);
        if (CurrentCycleIndex == INDEX_NONE) CurrentCycleIndex = 0;
    }
    else
    {
        FoundInteractiveActors = NearbyActors;
    }

    CurrentCycleIndex = (CurrentCycleIndex + Direction + FoundInteractiveActors.Num()) % FoundInteractiveActors.Num();
    SelectedInteractiveActor = FoundInteractiveActors[CurrentCycleIndex];
}

bool UKeyboardInteractiveActorHandler::IsActorReachable(AActor* TargetActor)
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    AActor* MyOwner = GetOwner();

    if (!NavSys || !TargetActor || !MyOwner) return false;

    const ANavigationData* NavData = Cast<ANavigationData>(NavSys->GetMainNavData());
    if (!NavData) return false;

    FVector StartLoc = MyOwner->GetActorLocation();
    FVector EndLoc = TargetActor->GetActorLocation();

    // Prefer the actor's designated move-to location over its root for path queries.
    if (TargetActor->Implements<UInteractableInterface>())
    {
        USceneComponent* MoveToComp = IInteractableInterface::Execute_GetInteractionMoveToComponent(TargetActor);
        if (MoveToComp)
        {
            EndLoc = MoveToComp->GetComponentLocation();
        }
    }

    FPathFindingQuery Query;
    Query.Owner = MyOwner;
    Query.NavData = NavData;
    Query.StartLocation = StartLoc;
    Query.EndLocation = EndLoc;
    Query.QueryFilter = NavData->GetDefaultQueryFilter();

    return NavSys->TestPathSync(Query);
}

void UKeyboardInteractiveActorHandler::SortFoundActors()
{
    FVector ForwardDir = GetOwner()->GetActorForwardVector();
    FVector StartLoc = GetOwner()->GetActorLocation();

    FoundInteractiveActors.Sort([ForwardDir, StartLoc](const AActor& A, const AActor& B) {
        float DotA = FVector::DotProduct(ForwardDir, (A.GetActorLocation() - StartLoc).GetSafeNormal());
        float DotB = FVector::DotProduct(ForwardDir, (B.GetActorLocation() - StartLoc).GetSafeNormal());
        return DotA > DotB;
    });
}

void UKeyboardInteractiveActorHandler::DrawInteractionDebugCone()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector StartLoc = Owner->GetActorLocation();
    FVector ForwardDir = Owner->GetActorForwardVector();

    DrawDebugCone(GetWorld(), StartLoc, ForwardDir, InteractiveConeRange,
        FMath::DegreesToRadians(InteractiveConeAngle * 0.5f),
        FMath::DegreesToRadians(InteractiveConeAngle * 0.5f),
        12, FColor::Yellow, false, -1, 0, 1.0f);

    for (AActor* Actor : FoundInteractiveActors)
    {
        FColor LineColor = (Actor == SelectedInteractiveActor) ? FColor::Cyan : FColor::Orange;
        DrawDebugLine(GetWorld(), StartLoc, Actor->GetActorLocation(), LineColor, false, -1, 0, 2.0f);
    }
}

void UKeyboardInteractiveActorHandler::DrawInteractionDebugSphere()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Center = Owner->GetActorLocation();
    FColor SphereColor = bManualCycleActive ? FColor::Green : FColor::Red;
    DrawDebugSphere(GetWorld(), Center, SelectionRadius, 32, SphereColor, false, -1.0f, 0, 2.0f);

    for (AActor* IA : FoundInteractiveActors)
    {
        if (IA)
        {
            FColor LineColor = (IA == SelectedInteractiveActor) ? FColor::Green : FColor::Blue;
            float LineThickness = (IA == SelectedInteractiveActor) ? 4.0f : 1.0f;
            DrawDebugLine(GetWorld(), Center, IA->GetActorLocation(), LineColor, false, -1.0f, 0, LineThickness);
        }
    }
}
