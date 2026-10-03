#include "Validation/BlueprintNamingRule.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Blueprint.h"
#include "Materials/Material.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBlueprintNamingRuleContractTest,
	"PipelineToolkit.Validation.BlueprintNamingRule",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBlueprintNamingRuleContractTest::RunTest(const FString& Parameters)
{
	const struct
	{
		const TCHAR* Name;
		const TCHAR* Prefix;
		bool bBlueprint;
		EValidationStatus Status;
		const TCHAR* SuggestedName;
	} Cases[] = {
		{TEXT("BP_Contract"), TEXT("BP_"), true, EValidationStatus::Passed, TEXT("")},
		{TEXT("Contract"), TEXT("BP_"), true, EValidationStatus::Failed, TEXT("BP_Contract")},
		{TEXT("Test_Contract"), TEXT("Test_"), true, EValidationStatus::Passed, TEXT("")},
		{TEXT("BP_Contract"), TEXT("Test_"), true, EValidationStatus::Failed, TEXT("Test_BP_Contract")},
		{TEXT("Contract"), TEXT("BP_"), false, EValidationStatus::Skipped, TEXT("")}
	};

	for (const auto& Item : Cases)
	{
		const FString PackageName = FString(TEXT("/Game/ContractMetadata/")) + Item.Name;
		const FAssetData Asset(FName(*PackageName), FName(TEXT("/Game/ContractMetadata")),
			FName(Item.Name), Item.bBlueprint ? UBlueprint::StaticClass()->GetClassPathName()
				: UMaterial::StaticClass()->GetClassPathName());
		const FValidationResult Result = FBlueprintNamingRule::Evaluate(Asset, Item.Prefix);
		const FString Label = FString::Printf(TEXT("%s with %s: "), Item.Name, Item.Prefix);
		TestTrue(Label + TEXT("status"), Result.Status == Item.Status);
		TestEqual(Label + TEXT("asset name"), Result.AssetName, FName(Item.Name));
		TestEqual(Label + TEXT("object path"), Result.AssetObjectPath.ToString(),
			PackageName + TEXT(".") + Item.Name);
		TestEqual(Label + TEXT("rule identity"), Result.RuleId, FName(TEXT("BlueprintNaming")));
		TestEqual(Label + TEXT("fix eligibility"), Result.bCanAutoFix, Item.Status == EValidationStatus::Failed);
		TestEqual(Label + TEXT("suggested name"), Result.SuggestedAssetName, FName(Item.SuggestedName));
	}
	return true;
}

#endif
