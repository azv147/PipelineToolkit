#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Validation/ValidationResult.h"

class FBlueprintNamingRule
{
public:
    static FValidationResult Evaluate(const FAssetData& AssetData);
};