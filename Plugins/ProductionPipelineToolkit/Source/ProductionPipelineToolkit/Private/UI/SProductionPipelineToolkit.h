#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"

#include "Validation/ValidationResult.h"

class SProductionPipelineToolkit : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SProductionPipelineToolkit)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	FReply OnScanClicked();

	void SetStatus(const FText& NewStatus);

	TSharedPtr<class STextBlock> StatusText;

	TSharedPtr<SVerticalBox> ResultsContainer;
	TArray<FValidationResult> ValidationResults;
};