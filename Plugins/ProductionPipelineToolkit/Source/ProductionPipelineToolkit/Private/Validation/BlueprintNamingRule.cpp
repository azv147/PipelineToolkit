#include "Validation/BlueprintNamingRule.h"

#include "Engine/Blueprint.h"

FValidationResult FBlueprintNamingRule::Evaluate(const FAssetData& AssetData)
{
    FValidationResult Result;

    Result.AssetName = AssetData.AssetName;
    Result.AssetObjectPath =
        FSoftObjectPath(AssetData.GetObjectPathString());

    Result.RuleId = FName(TEXT("BlueprintNaming"));

    const bool bIsBlueprint =
        AssetData.AssetClassPath ==
        UBlueprint::StaticClass()->GetClassPathName();

    if (!bIsBlueprint)
    {
        Result.Status = EValidationStatus::Skipped;
        Result.Message =
            TEXT("Rule does not apply to this asset type.");

        return Result;
    }

    if (AssetData.AssetName.ToString().StartsWith(TEXT("BP_")))
    {
        Result.Status = EValidationStatus::Passed;
        Result.Message =
            TEXT("Blueprint name follows the BP_ naming convention.");

        return Result;
    }

    Result.Status = EValidationStatus::Failed;
    Result.Message =
        TEXT("Blueprint asset name must start with BP_.");

    return Result;
}