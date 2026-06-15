// Copyright Jordan Chiquet 2025

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "RichTextBlockColorDecorator.generated.h"

/**
 * Rich text decorator that adds inline hex color support via a <color hex="RRGGBB"> tag.
 * Used in dialogue UI to allow per-run color overrides without switching style sets.
 */
UCLASS()
class WETDISCO2_API URichTextBlockColorDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:
	URichTextBlockColorDecorator(const FObjectInitializer& ObjectInitializer);

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
};
