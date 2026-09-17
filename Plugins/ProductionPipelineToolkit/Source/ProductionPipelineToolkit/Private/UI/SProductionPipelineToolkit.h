#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

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
};