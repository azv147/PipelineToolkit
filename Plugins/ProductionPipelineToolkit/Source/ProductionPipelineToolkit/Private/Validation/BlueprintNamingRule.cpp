#include "Validation/BlueprintNamingRule.h"

#include "Engine/Blueprint.h"

FValidationResult FBlueprintNamingRule::Evaluate(const FAssetData& AssetData,
	                                             const FString& RequiredPrefix)
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


    // if (AssetData.AssetName.ToString().StartsWith(TEXT("BP_")))
    // {
    //     Result.Status = EValidationStatus::Passed;
    //     Result.Message =
    //         TEXT("Blueprint name follows the BP_ naming convention.");

    //     return Result;
    // }
    // Result.Status = EValidationStatus::Failed;
    // Result.Message =
    //     TEXT("Blueprint asset name must start with BP_.");
    const FString AssetName = AssetData.AssetName.ToString();

    if (AssetName.StartsWith(RequiredPrefix))
    {
        Result.Status = EValidationStatus::Passed;

        Result.Message = FString::Printf(
            TEXT("Blueprint name follows the %s naming convention."),
            *RequiredPrefix
        );
    }
    // else
    // {
    //     Result.Status = EValidationStatus::Failed;

    //     Result.Message = FString::Printf(
    //         TEXT("Blueprint asset name must start with %s."),
    //         *RequiredPrefix
    //     );
    // }
    else
    {
        Result.Status = EValidationStatus::Failed;

        Result.Message = FString::Printf(
            TEXT("Blueprint asset name must start with %s."),
            *RequiredPrefix
        );

        const FString SuggestedName =
            RequiredPrefix + AssetName;

        Result.bCanAutoFix = true;
        Result.SuggestedAssetName = FName(*SuggestedName);
    }

    return Result;
}