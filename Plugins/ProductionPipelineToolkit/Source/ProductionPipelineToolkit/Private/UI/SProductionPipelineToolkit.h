#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"

#include "Validation/ValidationResult.h"
#include "Validation/ValidationRun.h"

class STextBlock;
class SVerticalBox;
class SEditableTextBox;
class SButton;

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

	void ClearValidationPresentation();

	FReply OnAutoFixClicked(FValidationResult Result);

	FReply OnRevertAutoFixClicked();

private:
	TSharedPtr<SButton> RevertFixButton;
	FSoftObjectPath LastAutoFixOriginalPath;
	FSoftObjectPath LastAutoFixFixedPath;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> SummaryText;

	TSharedPtr<SEditableTextBox> ScanScopeTextBox;
	TSharedPtr<SEditableTextBox> BlueprintPrefixTextBox;
	TSharedPtr<SVerticalBox> ResultsContainer;

	// TArray<FValidationResult> ValidationResults;
	FValidationRun ValidationRun;
};