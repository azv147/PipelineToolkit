#pragma once

#include "Commandlets/Commandlet.h"
#include "ValidateAssetsCommandlet.generated.h"

UCLASS()
class UValidateAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UValidateAssetsCommandlet();
	virtual int32 Main(const FString& Params) override;
};
