// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InteractiveDialogueActor.generated.h"

/**
 * Base class for any actor that can initiate dialogue (people, interactable "things").
 * Mesh setup (skeletal vs static root) is left to Blueprint children so each can
 * choose the appropriate visual representation.
 */
UCLASS(Abstract)
class WETDISCO2_API AInteractiveDialogueActor : public AInteractiveActor
{
    GENERATED_BODY()

public:
    AInteractiveDialogueActor();

    /**
     * Other dialogue actors linked to this one. When this actor starts dialogue,
     * linked actors are pulled in as well, so the dialogue graph can reference
     * everyone present in the scene.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
    TArray<AInteractiveDialogueActor*> LinkedDialogueActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
    FName DialogueParticipantName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
    FText DialogueParticipantDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
    FLinearColor DialogueDisplayNameAndBorderColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
    UTexture2D* DialogueParticipantIcon;

    /** C++ provides a logging stub; actual dialogue trigger logic lives in Blueprint. */
    virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;
};
