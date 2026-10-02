#pragma once

#include "Validation/ValidationConfig.h"
#include "Validation/ValidationRun.h"

class PRODUCTIONPIPELINETOOLKIT_API FValidationRunner
{
public:
	// False means execution failed, not that a rule reported a failed asset.
	// Each call clears OutRun and the optional error before starting.
	static bool Run(
		const FValidationConfig& Config,
		FValidationRun& OutRun,
		FString* OutErrorMessage = nullptr);
};
