#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

enum class EValidationStatus : uint8
{
    Passed,
    Failed,
    Skipped
};

struct FValidationResult
{
    FName AssetName;

    FSoftObjectPath AssetObjectPath;

    FName RuleId;

    EValidationStatus Status = EValidationStatus::Skipped;

    FString Message;

    bool bCanAutoFix = false;

    FName SuggestedAssetName;
};