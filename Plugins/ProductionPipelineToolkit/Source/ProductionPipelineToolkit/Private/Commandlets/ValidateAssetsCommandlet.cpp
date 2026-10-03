#include "ValidateAssetsCommandlet.h"

#include "Misc/Paths.h"
#include "Validation/ValidationReportWriter.h"
#include "Validation/ValidationRunner.h"

DEFINE_LOG_CATEGORY_STATIC(LogPipelineValidation, Log, All);

UValidateAssetsCommandlet::UValidateAssetsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
	// Preserve our 0/1/2 contract even when other modules log errors at startup.
	UseCommandletResultAsExitCode = true;
	HelpDescription = TEXT("Validate asset naming without modifying assets. Exit codes: 0 passed, 1 rule failures, 2 execution/report error.");
	HelpUsage = TEXT("UnrealEditor-Cmd.exe <Project> -run=ValidateAssets [-Scope=/Game/ValidationTest] [-BlueprintPrefix=BP_] [-ReportPath=Saved/ValidationReports/ValidationReport.json]");
}

int32 UValidateAssetsCommandlet::Main(const FString& Params)
{
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> Arguments;
	ParseCommandLine(*Params, Tokens, Switches, Arguments);

	FValidationConfig Config{TEXT("/Game/ValidationTest"), TEXT("BP_")};
	FString ReportPath = TEXT("Saved/ValidationReports/ValidationReport.json");

	// Engine switches are allowed; our value arguments require the key=value form.
	for (const FString& Switch : Switches)
	{
		if (Switch.Equals(TEXT("Scope"), ESearchCase::IgnoreCase) ||
			Switch.Equals(TEXT("BlueprintPrefix"), ESearchCase::IgnoreCase) ||
			Switch.Equals(TEXT("ReportPath"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogPipelineValidation, Error, TEXT("-%s requires =<value>."), *Switch);
			return 2;
		}
	}

	for (const auto& Argument : Arguments)
	{
		if (Argument.Key.Equals(TEXT("Scope"), ESearchCase::IgnoreCase))
		{
			Config.Scope = Argument.Value;
		}
		else if (Argument.Key.Equals(TEXT("BlueprintPrefix"), ESearchCase::IgnoreCase))
		{
			Config.BlueprintPrefix = Argument.Value;
		}
		else if (Argument.Key.Equals(TEXT("ReportPath"), ESearchCase::IgnoreCase))
		{
			ReportPath = Argument.Value;
		}
	}

	ReportPath.TrimStartAndEndInline();
	if (ReportPath.IsEmpty())
	{
		UE_LOG(LogPipelineValidation, Error, TEXT("ReportPath is empty."));
		return 2;
	}
	if (FPaths::IsRelative(ReportPath))
	{
		ReportPath = FPaths::ProjectDir() / ReportPath;
	}
	ReportPath = FPaths::ConvertRelativePathToFull(ReportPath);

	FValidationRun Run;
	FString Error;
	if (!FValidationRunner::Run(Config, Run, &Error))
	{
		UE_LOG(LogPipelineValidation, Error, TEXT("Validation execution failed: %s. No report written."), *Error);
		return 2;
	}
	if (!FValidationReportWriter::WriteJson(Run, ReportPath, &Error))
	{
		UE_LOG(LogPipelineValidation, Error, TEXT("Report writing failed: %s"), *Error);
		return 2;
	}

	const int32 ExitCode = Run.FailedCount > 0 ? 1 : 0;
	UE_LOG(LogPipelineValidation, Display, TEXT("Report=%s | Assets=%d | Passed=%d | Failed=%d | Skipped=%d | ExitCode=%d"),
		*ReportPath, Run.AssetCount, Run.PassedCount, Run.FailedCount, Run.SkippedCount, ExitCode);
	return ExitCode;
}
