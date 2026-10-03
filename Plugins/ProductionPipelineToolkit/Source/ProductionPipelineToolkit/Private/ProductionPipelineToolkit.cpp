// Copyright (c) 2026 [azv147]
// SPDX-License-Identifier: MIT

#include "ProductionPipelineToolkit.h"
#include "UI/SProductionPipelineToolkit.h"

#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FProductionPipelineToolkitModule"

void FProductionPipelineToolkitModule::StartupModule()
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("ProductionPipelineToolkit: StartupModule() called.")
	);

	if (IsRunningCommandlet())
	{
		return;
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		"ProductionPipelineToolkitTab",
		FOnSpawnTab::CreateRaw(
			this,
			&FProductionPipelineToolkitModule::OnSpawnTab
		)
	)
	.SetDisplayName(
		LOCTEXT(
			"ProductionPipelineToolkitTabTitle",
			"Production Pipeline Toolkit"
		)
	)
	.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(
			this,
			&FProductionPipelineToolkitModule::RegisterMenus
		)
	);
}

void FProductionPipelineToolkitModule::ShutdownModule()
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("ProductionPipelineToolkit: ShutdownModule() called.")
	);

	if (IsRunningCommandlet())
	{
		return;
	}

	UToolMenus::UnRegisterStartupCallback(this);

	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(
			"ProductionPipelineToolkitTab"
		);
	}
}

TSharedRef<SDockTab> FProductionPipelineToolkitModule::OnSpawnTab(
	const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SNew(SProductionPipelineToolkit)
	];
}

void FProductionPipelineToolkitModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu =
		UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");

	if (!Menu)
	{
		return;
	}

	FToolMenuSection& Section =
		Menu->FindOrAddSection("PipelineToolkit");

	Section.AddMenuEntry(
		"OpenProductionPipelineToolkit",
		LOCTEXT(
			"OpenToolkitLabel",
			"Production Pipeline Toolkit"
		),
		LOCTEXT(
			"OpenToolkitTooltip",
			"Open the Production Pipeline Toolkit"
		),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda(
				[]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(
						FTabId("ProductionPipelineToolkitTab")
					);
				}
			)
		)
	);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(
	FProductionPipelineToolkitModule,
	ProductionPipelineToolkit
)
