#include "SProductionPipelineToolkit.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
// #include "ScopedTransaction.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

#include "Validation/ValidationResult.h"
#include "Validation/ValidationReportWriter.h"
#include "Validation/ValidationRunner.h"

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
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SButton)
			.Text(
				FText::FromString(
					TEXT("Export JSON")
				)
			)
			.OnClicked(
				this,
				&SProductionPipelineToolkit::OnExportJsonClicked
			)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SAssignNew(RevertFixButton, SButton)
			.Text(FText::FromString(TEXT("Revert Last Fix")))
			.IsEnabled(false)
			.OnClicked(
				this,
				&SProductionPipelineToolkit::OnRevertAutoFixClicked
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

	FValidationConfig Config;
	Config.Scope = ScanScopeTextBox.IsValid()
		? ScanScopeTextBox->GetText().ToString()
		: TEXT("/Game/ValidationTest");
	Config.BlueprintPrefix = BlueprintPrefixTextBox.IsValid()
		? BlueprintPrefixTextBox->GetText().ToString()
		: TEXT("BP_");

	FString ErrorMessage;
	if (!FValidationRunner::Run(Config, ValidationRun, &ErrorMessage))
	{
		ClearValidationPresentation();
		SetStatus(FText::FromString(TEXT("Status: ") + ErrorMessage));
		return FReply::Handled();
	}

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
	// Refresh Slate result list
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

	SetStatus(FText::FromString(TEXT("Status: Scan Complete")));

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
    } else
	{
		LastAutoFixOriginalPath =
			FSoftObjectPath(SourceAssetPath);

		LastAutoFixFixedPath =
			FSoftObjectPath(DestinationAssetPath);

		if (RevertFixButton.IsValid())
		{
			RevertFixButton->SetEnabled(true);
		}
		SetStatus(
			FText::FromString(TEXT("Status: Auto-Fix Complete"))
		);
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

FReply SProductionPipelineToolkit::OnRevertAutoFixClicked()
{
    if (!LastAutoFixOriginalPath.IsValid() ||
        !LastAutoFixFixedPath.IsValid())
    {
        return FReply::Handled();
    }

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
            FText::FromString(
                TEXT("Status: Asset Subsystem Unavailable")
            )
        );

        return FReply::Handled();
    }

    const FString FixedAssetPath =
        LastAutoFixFixedPath.ToString();

    const FString OriginalAssetPath =
        LastAutoFixOriginalPath.ToString();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Auto-Fix revert requested: %s -> %s"),
        *FixedAssetPath,
        *OriginalAssetPath
    );

    const bool bSuccess =
        AssetSubsystem->RenameAsset(
            FixedAssetPath,
            OriginalAssetPath
        );

    if (!bSuccess)
    {
        SetStatus(
            FText::FromString(TEXT("Status: Revert Failed"))
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Auto-Fix revert failed: %s -> %s"),
            *FixedAssetPath,
            *OriginalAssetPath
        );

        return FReply::Handled();
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Auto-Fix revert succeeded: %s -> %s"),
        *FixedAssetPath,
        *OriginalAssetPath
    );

    LastAutoFixOriginalPath.Reset();
    LastAutoFixFixedPath.Reset();

    if (RevertFixButton.IsValid())
    {
        RevertFixButton->SetEnabled(false);
    }

    OnScanClicked();

    SetStatus(
        FText::FromString(TEXT("Status: Auto-Fix Reverted"))
    );

    return FReply::Handled();
}

FReply SProductionPipelineToolkit::OnExportJsonClicked()
{
    if (ValidationRun.Scope.IsEmpty())
    {
        SetStatus(
            FText::FromString(
                TEXT("Status: No Validation Run")
            )
        );

        return FReply::Handled();
    }

    const FString ReportDirectory =
        FPaths::ProjectSavedDir() /
        TEXT("ValidationReports");

    const FString ReportPath =
        ReportDirectory /
        TEXT("ValidationReport.json");

    FString ErrorMessage;

    const bool bSuccess =
        FValidationReportWriter::WriteJson(
            ValidationRun,
            ReportPath,
            &ErrorMessage
        );

    if (!bSuccess)
    {
        SetStatus(
            FText::FromString(
                TEXT("Status: JSON Export Failed")
            )
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("JSON export failed: %s"),
            *ErrorMessage
        );

        return FReply::Handled();
    }

    SetStatus(
        FText::FromString(
            TEXT("Status: JSON Export Complete")
        )
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Validation JSON exported: %s"),
        *ReportPath
    );

    return FReply::Handled();
}