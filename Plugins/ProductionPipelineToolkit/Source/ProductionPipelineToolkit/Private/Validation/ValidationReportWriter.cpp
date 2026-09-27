#include "Validation/ValidationReportWriter.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
    FString ValidationStatusToString(
        EValidationStatus Status
    )
    {
        switch (Status)
        {
        case EValidationStatus::Passed:
            return TEXT("Passed");

        case EValidationStatus::Failed:
            return TEXT("Failed");

        case EValidationStatus::Skipped:
            return TEXT("Skipped");
        }

        return TEXT("Unknown");
    }
}

bool FValidationReportWriter::WriteJson(
    const FValidationRun& ValidationRun,
    const FString& FilePath,
    FString* OutErrorMessage
)
{
    const TSharedRef<FJsonObject> RootObject =
        MakeShared<FJsonObject>();

    RootObject->SetNumberField(
        TEXT("schema_version"),
        1
    );

    RootObject->SetStringField(
        TEXT("tool"),
        TEXT("ProductionPipelineToolkit")
    );

    RootObject->SetStringField(
        TEXT("scope"),
        ValidationRun.Scope
    );

    RootObject->SetNumberField(
        TEXT("asset_count"),
        ValidationRun.AssetCount
    );

    RootObject->SetNumberField(
        TEXT("passed"),
        ValidationRun.PassedCount
    );

    RootObject->SetNumberField(
        TEXT("failed"),
        ValidationRun.FailedCount
    );

    RootObject->SetNumberField(
        TEXT("skipped"),
        ValidationRun.SkippedCount
    );

    TArray<TSharedPtr<FJsonValue>> JsonResults;

    for (const FValidationResult& Result :
        ValidationRun.Results)
    {
        const TSharedRef<FJsonObject> ResultObject =
            MakeShared<FJsonObject>();

        ResultObject->SetStringField(
            TEXT("asset"),
            Result.AssetName.ToString()
        );

        ResultObject->SetStringField(
            TEXT("object_path"),
            Result.AssetObjectPath.ToString()
        );

        ResultObject->SetStringField(
            TEXT("rule"),
            Result.RuleId.ToString()
        );

        ResultObject->SetStringField(
            TEXT("status"),
            ValidationStatusToString(Result.Status)
        );

        ResultObject->SetStringField(
            TEXT("message"),
            Result.Message
        );

        ResultObject->SetBoolField(
            TEXT("can_auto_fix"),
            Result.bCanAutoFix
        );

        if (!Result.SuggestedAssetName.IsNone())
        {
            ResultObject->SetStringField(
                TEXT("suggested_asset_name"),
                Result.SuggestedAssetName.ToString()
            );
        }

        JsonResults.Add(
            MakeShared<FJsonValueObject>(ResultObject)
        );
    }

    RootObject->SetArrayField(
        TEXT("results"),
        JsonResults
    );

    FString JsonString;

    const TSharedRef<
        TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>
    > Writer =
        TJsonWriterFactory<
            TCHAR,
            TPrettyJsonPrintPolicy<TCHAR>
        >::Create(&JsonString);

    const bool bSerialized =
        FJsonSerializer::Serialize(
            RootObject,
            Writer
        );

    Writer->Close();

    if (!bSerialized)
    {
        if (OutErrorMessage)
        {
            *OutErrorMessage =
                TEXT("Failed to serialize ValidationRun to JSON.");
        }

        return false;
    }

    const FString Directory =
        FPaths::GetPath(FilePath);

    if (!Directory.IsEmpty())
    {
        IFileManager::Get().MakeDirectory(
            *Directory,
            true
        );
    }

    const bool bWritten =
        FFileHelper::SaveStringToFile(
            JsonString,
            *FilePath
        );

    if (!bWritten)
    {
        if (OutErrorMessage)
        {
            *OutErrorMessage =
                FString::Printf(
                    TEXT("Failed to write JSON report: %s"),
                    *FilePath
                );
        }

        return false;
    }

    return true;
}