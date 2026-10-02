#include "Validation/ValidationRunner.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FValidationRunnerFixtureTest,
	"PipelineToolkit.Validation.Runner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FValidationRunnerFixtureTest::RunTest(const FString& Parameters)
{
	FValidationConfig Config{TEXT(" /Game/ValidationTest "), TEXT(" BP_ ")};
	FValidationRun Run;
	FString Error = TEXT("Previous error");
	if (!TestTrue(TEXT("Fixture execution succeeds"), FValidationRunner::Run(Config, Run, &Error)))
	{
		return false;
	}

	TestTrue(TEXT("Success clears the previous error"), Error.IsEmpty());
	TestEqual(TEXT("Scope is trimmed"), Run.Scope, FString(TEXT("/Game/ValidationTest")));
	TestEqual(TEXT("Assets checked"), Run.AssetCount, 4);
	TestEqual(TEXT("Result count"), Run.Results.Num(), 4);
	TestEqual(TEXT("Passed"), Run.PassedCount, 2);
	TestEqual(TEXT("Failed"), Run.FailedCount, 1);
	TestEqual(TEXT("Skipped"), Run.SkippedCount, 1);

	const struct
	{
		const TCHAR* ObjectPath;
		EValidationStatus Status;
	} Expected[] = {
		{TEXT("/Game/ValidationTest/BP_PlayerTest.BP_PlayerTest"), EValidationStatus::Passed},
		{TEXT("/Game/ValidationTest/Test_Player.Test_Player"), EValidationStatus::Failed},
		{TEXT("/Game/ValidationTest/M_Test.M_Test"), EValidationStatus::Skipped},
		{TEXT("/Game/ValidationTest/Characters/BP_EnemyTest.BP_EnemyTest"), EValidationStatus::Passed}
	};

	for (const auto& Asset : Expected)
	{
		const FValidationResult* Result = Run.Results.FindByPredicate(
			[&Asset](const FValidationResult& Item)
			{
				return Item.AssetObjectPath.ToString() == Asset.ObjectPath;
			});
		if (TestNotNull(Asset.ObjectPath, Result))
		{
			TestTrue(Asset.ObjectPath, Result->Status == Asset.Status);
			TestEqual(TEXT("Rule identity"), Result->RuleId, FName(TEXT("BlueprintNaming")));
			TestEqual(TEXT("Fix eligibility"), Result->bCanAutoFix, Asset.Status == EValidationStatus::Failed);
			if (Result->bCanAutoFix)
			{
				TestEqual(TEXT("Suggested rename"), Result->SuggestedAssetName, FName(TEXT("BP_Test_Player")));
			}
		}
	}

	Config.BlueprintPrefix = TEXT("Test_");
	TestTrue(TEXT("Configured prefix executes without an error output"), FValidationRunner::Run(Config, Run));
	TestEqual(TEXT("Alternate prefix passed"), Run.PassedCount, 1);
	TestEqual(TEXT("Alternate prefix failed"), Run.FailedCount, 2);
	TestEqual(TEXT("Alternate prefix skipped"), Run.SkippedCount, 1);
	TestEqual(TEXT("Repeated execution replaces results"), Run.Results.Num(), 4);

	AddExpectedMessagePlain(TEXT("Scan scope is empty."), ELogVerbosity::Warning);
	Config.Scope = TEXT("   ");
	TestFalse(TEXT("Blank scope fails execution"), FValidationRunner::Run(Config, Run, &Error));
	TestEqual(TEXT("Scope error"), Error, FString(TEXT("Invalid Scan Scope")));
	TestTrue(TEXT("Failure clears the previous scope"), Run.Scope.IsEmpty());
	TestEqual(TEXT("Failure clears previous results"), Run.Results.Num(), 0);
	TestEqual(TEXT("Failure clears previous counts"), Run.AssetCount + Run.PassedCount + Run.FailedCount + Run.SkippedCount, 0);

	AddExpectedMessagePlain(TEXT("Blueprint naming prefix is empty."), ELogVerbosity::Warning);
	Config.Scope = TEXT("/Game/ValidationTest");
	Config.BlueprintPrefix = TEXT("   ");
	TestFalse(TEXT("Blank prefix fails execution"), FValidationRunner::Run(Config, Run, &Error));
	TestEqual(TEXT("Prefix error"), Error, FString(TEXT("Invalid Rule Configuration")));
	TestTrue(TEXT("Invalid configuration leaves no exportable run"), Run.Scope.IsEmpty());
	TestEqual(TEXT("Invalid configuration leaves no results"), Run.Results.Num(), 0);
	return true;
}

#endif
