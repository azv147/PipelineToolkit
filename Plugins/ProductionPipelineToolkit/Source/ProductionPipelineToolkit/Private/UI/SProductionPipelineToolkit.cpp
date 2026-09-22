#include "SProductionPipelineToolkit.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"

#include "Validation/ValidationResult.h"
#include "Validation/BlueprintNamingRule.h"

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

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Validation Results")))
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10.0f)
		[
			SAssignNew(ResultsContainer, SVerticalBox)
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

	// const FTopLevelAssetPath BlueprintClassPath =
    // UBlueprint::StaticClass()->GetClassPathName();
	ValidationResults.Reset();
    for (const FAssetData& AssetData : AssetDataList)
    {
		const FValidationResult Result = FBlueprintNamingRule::Evaluate(AssetData);
		ValidationResults.Add(Result);

		const TCHAR* ResultStatusText = TEXT("UNKNOWN");

		switch (Result.Status)
		{
		case EValidationStatus::Passed:
			ResultStatusText = TEXT("PASS");
			break;

		case EValidationStatus::Failed:
			ResultStatusText = TEXT("FAIL");
			break;

		case EValidationStatus::Skipped:
			ResultStatusText = TEXT("SKIP");
			break;
		}

		UE_LOG(
			LogTemp,
			Display,
			TEXT("[%s] Rule=%s | Asset=%s | Message=%s"),
			ResultStatusText,
			*Result.RuleId.ToString(),
			*Result.AssetName.ToString(),
			*Result.Message
		);

		// FValidationResult Result;

		// Result.AssetName = AssetData.AssetName;
		// Result.AssetObjectPath = FSoftObjectPath(AssetData.GetObjectPathString());
		// Result.RuleId = FName(TEXT("BlueprintNaming"));

		// const bool bIsBlueprint =
		// 	AssetData.AssetClassPath ==
		// 	UBlueprint::StaticClass()->GetClassPathName();

		// if (!bIsBlueprint)
		// {
		// 	Result.Status = EValidationStatus::Skipped;
		// 	Result.Message = TEXT("Rule does not apply to this asset type.");
		// }
		// else if (AssetData.AssetName.ToString().StartsWith(TEXT("BP_")))
		// {
		// 	Result.Status = EValidationStatus::Passed;
		// 	Result.Message = TEXT("Blueprint name follows the BP_ naming convention.");
		// }
		// else
		// {
		// 	Result.Status = EValidationStatus::Failed;
		// 	Result.Message = TEXT("Blueprint asset name must start with BP_.");
		// }

		// const TCHAR* ResultStatusText = TEXT("UNKNOWN");

		// switch (Result.Status)
		// {
		// case EValidationStatus::Passed:
		// 	ResultStatusText = TEXT("PASS");
		// 	break;

		// case EValidationStatus::Failed:
		// 	ResultStatusText = TEXT("FAIL");
		// 	break;

		// case EValidationStatus::Skipped:
		// 	ResultStatusText = TEXT("SKIP");
		// 	break;
		// }

		// UE_LOG(
		// 	LogTemp,
		// 	Display,
		// 	TEXT("[%s] Rule=%s | Asset=%s | Message=%s"),
		// 	ResultStatusText,
		// 	*Result.RuleId.ToString(),
		// 	*Result.AssetName.ToString(),
		// 	*Result.Message
		// );

		// const bool bIsBlueprint =
		// AssetData.AssetClassPath == BlueprintClassPath;

		// if (!bIsBlueprint)
		// {
		// 	UE_LOG(
		// 	LogTemp,
		// 	Display,
		// 	TEXT("[SKIP] Rule=BlueprintNaming | Asset=%s | Class=%s"),
		// 	*AssetData.AssetName.ToString(),
		// 	*AssetData.AssetClassPath.ToString()
		// 	);

		// 	continue;
		// }

		// const bool bValidName =
		// AssetData.AssetName.ToString().StartsWith(TEXT("BP_"));

		// UE_LOG(
		// 	LogTemp,
		// 	Display,
		// 	TEXT("[%s] Rule=BlueprintNaming | Asset=%s | ObjectPath=%s"),
		// 	bValidName ? TEXT("PASS") : TEXT("FAIL"),
		// 	*AssetData.AssetName.ToString(),
		// 	*AssetData.GetObjectPathString()
		// );
        // UE_LOG(
        //     LogTemp,
        //     Display,
        //     TEXT("Asset: Name=%s | Class=%s | PackagePath=%s | ObjectPath=%s"),
        //     *AssetData.AssetName.ToString(),
        //     *AssetData.AssetClassPath.ToString(),
        //     *AssetData.PackagePath.ToString(),
        //     *AssetData.GetObjectPathString()
        // );
    }
	if (ResultsContainer.IsValid())
	{
		ResultsContainer->ClearChildren();

		for (const FValidationResult& Result : ValidationResults)
		{
			const TCHAR* SStatusText = TEXT("UNKNOWN");

			switch (Result.Status)
			{
			case EValidationStatus::Passed:
				SStatusText = TEXT("PASS");
				break;

			case EValidationStatus::Failed:
				SStatusText = TEXT("FAIL");
				break;

			case EValidationStatus::Skipped:
				SStatusText = TEXT("SKIP");
				break;
			}

			const FString RowText = FString::Printf(
				TEXT("%s    %s    %s"),
				*Result.AssetName.ToString(),
				*Result.RuleId.ToString(),
				SStatusText
			);

			ResultsContainer->AddSlot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(RowText))
			];
		}
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Validation Results: %d"),
		ValidationResults.Num()
	);

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