// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "wetDisco2Character.generated.h"

/** The player character. Owns the top-down camera rig and exposes dialogue participant
 *  data so the player can be referenced as a participant in Articy dialogue graphs. */
UCLASS(Blueprintable)
class AwetDisco2Character : public ACharacter
{
	GENERATED_BODY()

public:
	AwetDisco2Character();

	virtual void Tick(float DeltaSeconds) override;

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
	FName DialogueParticipantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
	FText DialogueParticipantDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dlg")
	UTexture2D* DialogueParticipantIcon;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};
