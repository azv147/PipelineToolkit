#include "SProductionPipelineToolkit.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
// #include "ScopedTransaction.h"
#include "Misc/PackageName.h"

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
		.Padding(10.0f, 10.0f, 10.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Scan Scope")))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SAssignNew(ScanScopeTextBox, SEditableTextBox)
			.Text(
				FText::FromString(TEXT("/Game/ValidationTest"))
			)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 4.0f, 10.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(
				FText::FromString(
					TEXT("Blueprint Naming Prefix")
				)
			)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SAssignNew(BlueprintPrefixTextBox, SEditableTextBox)
			.Text(
				FText::FromString(TEXT("BP_"))
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
			.Text(FText::FromString(TEXT("Validation Summary")))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f)
		[
			SAssignNew(SummaryText, STextBlock)
			.Text(
				FText::FromString(
					TEXT("Scope: -\n")
					TEXT("Assets Checked: 0\n")
					TEXT("Passed: 0    Failed: 0    Skipped: 0")
				)
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
	SetStatus(FText::FromString(TEXT("Status: Scanning...")));
	FString ScanPath = TEXT("/Game/ValidationTest");
	if (ScanScopeTextBox.IsValid())
	{
		ScanPath = ScanScopeTextBox->GetText().ToString();
	}
	ScanPath.TrimStartAndEndInline();

	// ------------------------------------------------------------
	// 1. Update UI state
	// ------------------------------------------------------------

	if (StatusText.IsValid())
	{
		StatusText->SetText(
			FText::FromString(TEXT("Scanning..."))
		);
	}
	if (ScanPath.IsEmpty())
	{
		ClearValidationPresentation();
		SetStatus(
			FText::FromString(TEXT("Status: Invalid Scan Scope"))
		);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Scan scope is empty.")
		);
		return FReply::Handled();
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	AssetRegistry.WaitForCompletion();

	TArray<FAssetData> AssetDataList;

	// const bool bSuccess = AssetRegistry.GetAssetsByPath(
	// 	FName(*ScanPath),
	// 	AssetDataList,
	// 	true,   // Recursive
	// 	true    // Include only on-disk assets
	// );
	const bool bSuccess = AssetRegistry.GetAssetsByPath(
		FName(*ScanPath),
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
		TEXT("Scan Path: %s"),
		*ScanPath
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Found %d assets"),
		AssetDataList.Num()
	);

	// ------------------------------------------------------------
	// 2. Handle Asset Registry failure
	// ------------------------------------------------------------

	if (!bSuccess)
	{
		ClearValidationPresentation();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Asset Registry query failed.")
		);

		if (StatusText.IsValid())
		{
			StatusText->SetText(
				FText::FromString(TEXT("Scan Failed"))
			);
			SetStatus(
				FText::FromString(TEXT("Status: Scan Failed"))
			);
		}
		return FReply::Handled();
	}

	ValidationRun = FValidationRun{};
	ValidationRun.Scope = ScanPath;

	FString BlueprintPrefix = TEXT("BP_");

		if (BlueprintPrefixTextBox.IsValid())
		{
			BlueprintPrefix =
				BlueprintPrefixTextBox->GetText().ToString();
		}

		BlueprintPrefix.TrimStartAndEndInline();

		if (BlueprintPrefix.IsEmpty())
		{
			ClearValidationPresentation();

			SetStatus(
				FText::FromString(
					TEXT("Status: Invalid Rule Configuration")
				)
			);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Blueprint naming prefix is empty.")
			);

			return FReply::Handled();
		}

	// ------------------------------------------------------------
	// 3. Start a new Validation Run
	// ------------------------------------------------------------


	ValidationRun.Results.Reserve(AssetDataList.Num());

	// ------------------------------------------------------------
	// 4. Evaluate every asset
	// ------------------------------------------------------------

	for (const FAssetData& AssetData : AssetDataList)
	{
		// const FValidationResult Result =
		// 	FBlueprintNamingRule::Evaluate(AssetData);
		const FValidationResult Result =
		FBlueprintNamingRule::Evaluate(
			AssetData,
			BlueprintPrefix
		);	

		ValidationRun.Results.Add(Result);

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

		default:
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

	}

	// ------------------------------------------------------------
	// 5. Calculate Validation Run summary
	// ------------------------------------------------------------

	ValidationRun.RecalculateSummary();

	if (SummaryText.IsValid())
	{
		const FString SummaryString = FString::Printf(
			TEXT("Scope: %s\n")
			TEXT("Assets Checked: %d\n")
			TEXT("Passed: %d    Failed: %d    Skipped: %d"),
			*ValidationRun.Scope,
			ValidationRun.AssetCount,
			ValidationRun.PassedCount,
			ValidationRun.FailedCount,
			ValidationRun.SkippedCount
		);

		SummaryText->SetText(
			FText::FromString(SummaryString)
		);
	}

	// ------------------------------------------------------------
	// 6. Refresh Slate result list
	// ------------------------------------------------------------

	if (ResultsContainer.IsValid())
	{
		ResultsContainer->ClearChildren();

		for (const FValidationResult& Result : ValidationRun.Results)
		{
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

			default:
				break;
			}

			const FString RowText = FString::Printf(
				TEXT("%s    %s    %s"),
				*Result.AssetName.ToString(),
				*Result.RuleId.ToString(),
				ResultStatusText
			);

			// ResultsContainer->AddSlot()
			// .AutoHeight()
			// .Padding(4.0f)
			// [
			// 	SNew(STextBlock)
			// 	.Text(FText::FromString(RowText))
			// ];
			//const FString MessageText = Result.Message;

			const FString MessageText =
				Result.Message.IsEmpty()
					? TEXT("No additional information.")
					: Result.Message;
			ResultsContainer->AddSlot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SVerticalBox)

				// Result header
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(RowText))
				]

				// Result message / issue detail
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(12.0f, 2.0f, 4.0f, 6.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(MessageText))
					.AutoWrapText(true)
				]
			];

			if (Result.bCanAutoFix)
			{
				ResultsContainer->AddSlot()
				.AutoHeight()
				.Padding(4.0f, 0.0f, 4.0f, 6.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Fix")))
					.OnClicked(
						FOnClicked::CreateSP(
							this,
							&SProductionPipelineToolkit::OnAutoFixClicked,
							Result
						)
					)
				];
			}
		}
	}

	// ------------------------------------------------------------
	// 7. Log Validation Run summary
	// ------------------------------------------------------------

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Validation Run Summary:")
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Scope=%s | Assets=%d | Passed=%d | Failed=%d | Skipped=%d"),
		*ValidationRun.Scope,
		ValidationRun.AssetCount,
		ValidationRun.PassedCount,
		ValidationRun.FailedCount,
		ValidationRun.SkippedCount
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Validation Results: %d"),
		ValidationRun.Results.Num()
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("====================================================")
	);

	// ------------------------------------------------------------
	// 8. Update UI state
	// ------------------------------------------------------------

	if (StatusText.IsValid())
	{
		StatusText->SetText(
			FText::FromString(TEXT("Scan Complete"))
		);
		SetStatus(
			FText::FromString(TEXT("Status: Scan Complete"))
		);
	}

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

void SProductionPipelineToolkit::ClearValidationPresentation()
{
	ValidationRun = FValidationRun{};

	if (ResultsContainer.IsValid())
	{
		ResultsContainer->ClearChildren();
	}

	if (SummaryText.IsValid())
	{
		SummaryText->SetText(
			FText::FromString(
				TEXT("Scope: -\n")
				TEXT("Assets Checked: 0\n")
				TEXT("Passed: 0    Failed: 0    Skipped: 0")
			)
		);
	}
}

FReply SProductionPipelineToolkit::OnAutoFixClicked(
    FValidationResult Result
)
{
    if (!Result.bCanAutoFix)
    {
        return FReply::Handled();
    }

    if (Result.Status != EValidationStatus::Failed)
    {
        return FReply::Handled();
    }

    if (Result.SuggestedAssetName.IsNone())
    {
        return FReply::Handled();
    }

    const FString SourceAssetPath =
        Result.AssetObjectPath.ToString();

    // const FString PackagePath =
    //     Result.AssetObjectPath.GetLongPackageName();

	const FString PackageName =
    	Result.AssetObjectPath.GetLongPackageName();

	const FString PackagePath =
		FPackageName::GetLongPackagePath(PackageName);

    if (SourceAssetPath.IsEmpty() || PackagePath.IsEmpty())
    {
        SetStatus(
            FText::FromString(TEXT("Status: Invalid Asset Path"))
        );

        return FReply::Handled();
    }

    const FString NewAssetName =
        Result.SuggestedAssetName.ToString();

	const FString DestinationPackageName =
		FString::Printf(
			TEXT("%s/%s"),
			*PackagePath,
			*NewAssetName
		);


    const FString DestinationAssetPath =
        FString::Printf(
            TEXT("%s.%s"),
            *DestinationPackageName,
            *NewAssetName
        );

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Auto-Fix requested: %s -> %s"),
		*SourceAssetPath,
		*DestinationAssetPath
	);

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Auto-Fix requested: %s -> %s"),
        *SourceAssetPath,
        *DestinationAssetPath
    );

    // FScopedTransaction Transaction(
    //     FText::FromString(
    //         TEXT("Fix Blueprint Asset Naming")
    //     )
    // );

    if (GEditor == nullptr)
    {
        SetStatus(
            FText::FromString(TEXT("Status: Editor Unavailable"))
        );

        return FReply::Handled();
    }

    UEditorAssetSubsystem* AssetSubsystem =
        GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

    if (AssetSubsystem == nullptr)
    {
        SetStatus(
            FText::FromString(TEXT("Status: Asset Subsystem Unavailable"))
        );

        return FReply::Handled();
    }

    const bool bSuccess =
        AssetSubsystem->RenameAsset(
            SourceAssetPath,
            DestinationAssetPath
        );

    if (!bSuccess)
    {
        SetStatus(
            FText::FromString(TEXT("Status: Auto-Fix Failed"))
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Auto-Fix failed: %s -> %s"),
            *SourceAssetPath,
            *DestinationAssetPath
        );

        return FReply::Handled();
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Auto-Fix succeeded: %s -> %s"),
        *SourceAssetPath,
        *DestinationAssetPath
    );

    OnScanClicked();

    SetStatus(
        FText::FromString(TEXT("Status: Auto-Fix Complete"))
    );

    return FReply::Handled();
}