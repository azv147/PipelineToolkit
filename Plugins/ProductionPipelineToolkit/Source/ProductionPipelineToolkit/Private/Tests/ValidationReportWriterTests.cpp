#include "Validation/ValidationReportWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FValidationReportWriterContractTest,
	"PipelineToolkit.Validation.ReportWriter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FValidationReportWriterContractTest::RunTest(const FString& Parameters)
{
	const FString Directory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()
		/ TEXT("Automation/ValidationContracts") / FGuid::NewGuid().ToString(EGuidFormats::Digits));
	if (!TestTrue(TEXT("Create isolated test directory"), IFileManager::Get().MakeDirectory(*Directory, true)))
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Only this invocation's GUID directory is owned by this test.
		TestTrue(TEXT("Clean isolated test directory"), IFileManager::Get().DeleteDirectory(*Directory, false, true));
	};

	auto ReadReport = [this](const FString& Path, TSharedPtr<FJsonObject>& Object)
	{
		FString Text;
		if (!TestTrue(TEXT("Read report file"), FFileHelper::LoadFileToString(Text, *Path)))
		{
			return false;
		}
		return TestTrue(TEXT("Parse report JSON"),
			FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Object) && Object.IsValid());
	};
	auto CheckString = [this](const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, const FString& Expected)
	{
		FString Actual;
		if (TestTrue(FString(Field) + TEXT(" exists as string"), Object->TryGetStringField(Field, Actual)))
		{
			TestEqual(Field, Actual, Expected);
		}
	};
	auto CheckNumber = [this](const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, int32 Expected)
	{
		double Actual = -1;
		if (TestTrue(FString(Field) + TEXT(" exists as number"), Object->TryGetNumberField(Field, Actual)))
		{
			TestEqual(Field, Actual, static_cast<double>(Expected));
		}
	};

	FValidationRun Run;
	Run.Scope = TEXT("/Game/ContractMetadata");
	const EValidationStatus Statuses[] = {EValidationStatus::Passed, EValidationStatus::Failed, EValidationStatus::Skipped};
	const TCHAR* Names[] = {TEXT("BP_Contract"), TEXT("Contract"), TEXT("M_Contract")};
	const TCHAR* StatusNames[] = {TEXT("Passed"), TEXT("Failed"), TEXT("Skipped")};
	const FString Message = TEXT("中文 \"quoted\" \\path\nnext line");
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FValidationResult Result;
		Result.AssetName = FName(Names[Index]);
		Result.AssetObjectPath = FSoftObjectPath(FString::Printf(TEXT("/Game/ContractMetadata/%s.%s"), Names[Index], Names[Index]));
		Result.RuleId = FName(TEXT("BlueprintNaming"));
		Result.Status = Statuses[Index];
		Result.Message = Message;
		Result.bCanAutoFix = Index == 1;
		if (Index == 1)
		{
			Result.SuggestedAssetName = FName(TEXT("BP_Contract"));
		}
		Run.Results.Add(Result);
	}
	Run.RecalculateSummary();
	const FString ReportPath = Directory / TEXT("Mixed report.json");
	FString Error;
	if (!TestTrue(TEXT("Write mixed run"), FValidationReportWriter::WriteJson(Run, ReportPath, &Error)))
	{
		AddError(Error);
		return false;
	}
	TSharedPtr<FJsonObject> Report;
	if (!ReadReport(ReportPath, Report))
	{
		return false;
	}
	CheckNumber(Report, TEXT("schema_version"), 1);
	CheckString(Report, TEXT("tool"), TEXT("ProductionPipelineToolkit"));
	CheckString(Report, TEXT("scope"), Run.Scope);
	CheckNumber(Report, TEXT("asset_count"), 3);
	CheckNumber(Report, TEXT("passed"), 1);
	CheckNumber(Report, TEXT("failed"), 1);
	CheckNumber(Report, TEXT("skipped"), 1);
	const TArray<TSharedPtr<FJsonValue>>* Results = nullptr;
	if (TestTrue(TEXT("Results array exists"), Report->TryGetArrayField(TEXT("results"), Results))
		&& TestEqual(TEXT("Three serialized results"), Results->Num(), 3))
	{
		for (int32 Index = 0; Index < 3; ++Index)
		{
			const TSharedPtr<FJsonObject>* Item = nullptr;
			if (!TestTrue(TEXT("Result is an object"), (*Results)[Index]->TryGetObject(Item)))
			{
				continue;
			}
			CheckString(*Item, TEXT("asset"), Names[Index]);
			CheckString(*Item, TEXT("object_path"), Run.Results[Index].AssetObjectPath.ToString());
			CheckString(*Item, TEXT("rule"), TEXT("BlueprintNaming"));
			CheckString(*Item, TEXT("status"), StatusNames[Index]);
			CheckString(*Item, TEXT("message"), Message);
			bool bCanFix = false;
			if (TestTrue(TEXT("Fix flag exists as boolean"), (*Item)->TryGetBoolField(TEXT("can_auto_fix"), bCanFix)))
			{
				TestEqual(TEXT("Fix flag"), bCanFix, Index == 1);
			}
			if (Index == 1)
			{
				CheckString(*Item, TEXT("suggested_asset_name"), TEXT("BP_Contract"));
			}
			else
			{
				TestFalse(TEXT("No absent suggestion serialized"), (*Item)->HasField(TEXT("suggested_asset_name")));
			}
		}
	}

	const FString EmptyPath = Directory / TEXT("Empty.json");
	if (TestTrue(TEXT("Write empty run without error output"), FValidationReportWriter::WriteJson(FValidationRun{}, EmptyPath))
		&& ReadReport(EmptyPath, Report))
	{
		CheckNumber(Report, TEXT("schema_version"), 1);
		CheckString(Report, TEXT("scope"), TEXT(""));
		for (const TCHAR* Field : {TEXT("asset_count"), TEXT("passed"), TEXT("failed"), TEXT("skipped")})
		{
			CheckNumber(Report, Field, 0);
		}
		if (TestTrue(TEXT("Empty results array exists"), Report->TryGetArrayField(TEXT("results"), Results)))
		{
			TestEqual(TEXT("Empty result count"), Results->Num(), 0);
		}
	}

	const FString Blocker = Directory / TEXT("WriteBlocker");
	if (TestTrue(TEXT("Create ordinary blocker file"), FFileHelper::SaveStringToFile(TEXT("block"), *Blocker)))
	{
		const FString BlockedPath = Blocker / TEXT("Report.json");
		Error.Reset();
		// This Writer reports failure through OutErrorMessage, without emitting a log.
		TestFalse(TEXT("Blocked report write fails"), FValidationReportWriter::WriteJson(Run, BlockedPath, &Error));
		TestTrue(TEXT("Write diagnostic identifies failure and target"),
			Error.Contains(TEXT("Failed to write JSON report:")) && Error.Contains(BlockedPath));
		TestFalse(TEXT("No blocked report created"), IFileManager::Get().FileExists(*BlockedPath));
	}
	return true;
}

#endif
