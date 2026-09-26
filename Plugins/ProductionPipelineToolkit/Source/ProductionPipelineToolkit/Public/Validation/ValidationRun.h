#pragma once

#include "CoreMinimal.h"
#include "Validation/ValidationResult.h"

struct FValidationRun
{
	FString Scope;

	TArray<FValidationResult> Results;

	int32 AssetCount = 0;
	int32 PassedCount = 0;
	int32 FailedCount = 0;
	int32 SkippedCount = 0;

	void RecalculateSummary()
	{
		AssetCount = Results.Num();

		PassedCount = 0;
		FailedCount = 0;
		SkippedCount = 0;

		for (const FValidationResult& Result : Results)
		{
			switch (Result.Status)
			{
			case EValidationStatus::Passed:
				++PassedCount;
				break;

			case EValidationStatus::Failed:
				++FailedCount;
				break;

			case EValidationStatus::Skipped:
				++SkippedCount;
				break;

			default:
				break;
			}
		}
	}
};