#pragma once

#include "CoreMinimal.h"
#include "Validation/ValidationRun.h"

struct FValidationReportWriter
{
    static bool WriteJson(
        const FValidationRun& ValidationRun,
        const FString& FilePath,
        FString* OutErrorMessage = nullptr
    );
};