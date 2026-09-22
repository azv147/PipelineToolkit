#include "SProductionPipelineToolkit.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"

void SProductionPipelineToolkit::Construct(
	const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f)
		[
			SNew(STextBlock)
			.Text(
				FText::FromString(
					TEXT("Production Pipeline Toolkit")
				)
			)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(
				FText::FromString(
					TEXT("Status: Ready")
				)
			)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f)
		[
			SNew(SButton)
			.Text(
				FText::FromString(
					TEXT("Scan")
				)
			)
			.OnClicked(
				this,
				&SProductionPipelineToolkit::OnScanClicked
			)
		]
	];
}

FReply SProductionPipelineToolkit::OnScanClicked()
{
	// SetStatus(
	// 	FText::FromString(
	// 		TEXT("Status: Scan Requested")
	// 	)
	// );

	FAssetRegistryModule& AssetRegistryModule =
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    AssetRegistry.WaitForCompletion();

    TArray<FAssetData> AssetDataList;

    const bool bSuccess = AssetRegistry.GetAssetsByPath(
        FName(TEXT("/Game/ValidationTest")),
        AssetDataList,
        true,   // Recursive
        true    // Include only on-disk assets
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT("========== ProductionPipelineToolkit Scan ==========")
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Scan Path: /Game/ValidationTest")
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Found %d assets"),
        AssetDataList.Num()
    );

    if (!bSuccess)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Asset Registry query failed.")
        );

        return FReply::Handled();
    }

    for (const FAssetData& AssetData : AssetDataList)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Asset: Name=%s | Class=%s | PackagePath=%s | ObjectPath=%s"),
            *AssetData.AssetName.ToString(),
            *AssetData.AssetClassPath.ToString(),
            *AssetData.PackagePath.ToString(),
            *AssetData.GetObjectPathString()
        );
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("====================================================")
    );


	return FReply::Handled();
}

void SProductionPipelineToolkit::SetStatus(
	const FText& NewStatus)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(NewStatus);
	}
}