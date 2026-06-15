// Copyright Jordan Chiquet 2025

#include "RichTextBlockColorDecorator.h"
#include "Components/RichTextBlock.h"
#include "Framework/Text/SlateTextRun.h"
#include "Framework/Text/ITextDecorator.h"

// Internal Slate decorator that handles parsing and rendering <color hex="RRGGBB"> tags.
class FRichTextColorDecorator : public ITextDecorator
{
public:
	FRichTextColorDecorator(URichTextBlock* InOwner) : Owner(InOwner) {}

	static TSharedRef<FRichTextColorDecorator> Create(URichTextBlock* InOwner)
	{
		return MakeShareable(new FRichTextColorDecorator(InOwner));
	}

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
	{
		return RunParseResult.Name == TEXT("color") && RunParseResult.MetaData.Contains(TEXT("hex"));
	}

	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override
	{
		FTextRange ModelRange;
		ModelRange.BeginIndex = InOutModelText->Len();
		*InOutModelText += OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex);
		ModelRange.EndIndex = InOutModelText->Len();

		// Inherit the owner's default style so font and size are preserved.
		FTextBlockStyle CurrentStyle = Owner ? Owner->GetDefaultTextStyle() : FTextBlockStyle();

		const FTextRange* HexRange = RunParseResult.MetaData.Find(TEXT("hex"));
		if (HexRange)
		{
			FString HexString = OriginalText.Mid(HexRange->BeginIndex, HexRange->Len());

			// FColor::FromHex is required here — FColor::InitFromString does not handle hex strings correctly.
			// Convert to linear sRGB so the result matches what the color picker produces.
			FColor SrgbColor = FColor::FromHex(HexString);
			CurrentStyle.SetColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(SrgbColor)));
		}

		// FRunInfo(Name) must be passed rather than the raw RunParseResult to avoid a Slate assert.
		FRunInfo RunInfo(RunParseResult.Name);
		return FSlateTextRun::Create(RunInfo, InOutModelText, CurrentStyle, ModelRange);
	}

private:
	URichTextBlock* Owner;
};

URichTextBlockColorDecorator::URichTextBlockColorDecorator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

TSharedPtr<ITextDecorator> URichTextBlockColorDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return FRichTextColorDecorator::Create(InOwner);
}
