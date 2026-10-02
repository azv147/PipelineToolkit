#include "Validation/ValidationRunner.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "UObject/ObjectRedirector.h"
#include "Validation/BlueprintNamingRule.h"

bool FValidationRunner::Run(
	const FValidationConfig& Config,
	FValidationRun& OutRun,
	FString* OutErrorMessage)
{
	OutRun = FValidationRun{};
	if (OutErrorMessage)
	{
		OutErrorMessage->Reset();
	}

	const auto Fail = [OutErrorMessage](const TCHAR* Error, const TCHAR* Detail)
	{
		if (OutErrorMessage)
		{
			*OutErrorMessage = Error;
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), Detail);
		return false;
	};

	FString Scope = Config.Scope;
	Scope.TrimStartAndEndInline();
	if (Scope.IsEmpty())
	{
		return Fail(TEXT("Invalid Scan Scope"), TEXT("Scan scope is empty."));
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.WaitForCompletion();

	TArray<FAssetData> AssetDataList;
	const bool bSuccess = AssetRegistry.GetAssetsByPath(
		FName(*Scope), AssetDataList, true, true); // Recursive, on-disk only.

	UE_LOG(LogTemp, Display, TEXT("Scan Path: %s"), *Scope);
	UE_LOG(LogTemp, Display, TEXT("Found %d assets"), AssetDataList.Num());
	UE_LOG(LogTemp, Display, TEXT("========== ProductionPipelineToolkit Scan =========="));

	if (!bSuccess)
	{
		return Fail(TEXT("Scan Failed"), TEXT("Asset Registry query failed."));
	}

	FString BlueprintPrefix = Config.BlueprintPrefix;
	BlueprintPrefix.TrimStartAndEndInline();
	if (BlueprintPrefix.IsEmpty())
	{
		return Fail(TEXT("Invalid Rule Configuration"), TEXT("Blueprint naming prefix is empty."));
	}

	OutRun.Scope = Scope;
	OutRun.Results.Reserve(AssetDataList.Num());
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetClassPath == UObjectRedirector::StaticClass()->GetClassPathName())
		{
			continue;
		}

		const FValidationResult Result = FBlueprintNamingRule::Evaluate(AssetData, BlueprintPrefix);
		OutRun.Results.Add(Result);

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

		UE_LOG(LogTemp, Display, TEXT("[%s] Rule=%s | Asset=%s | Message=%s"),
			ResultStatusText, *Result.RuleId.ToString(), *Result.AssetName.ToString(), *Result.Message);
	}

	OutRun.RecalculateSummary();
	UE_LOG(LogTemp, Display, TEXT("Validation Run Summary:"));
	UE_LOG(LogTemp, Display, TEXT("Scope=%s | Assets=%d | Passed=%d | Failed=%d | Skipped=%d"),
		*OutRun.Scope, OutRun.AssetCount, OutRun.PassedCount, OutRun.FailedCount, OutRun.SkippedCount);
	UE_LOG(LogTemp, Display, TEXT("Validation Results: %d"), OutRun.Results.Num());
	UE_LOG(LogTemp, Display, TEXT("===================================================="));
	return true;
}
