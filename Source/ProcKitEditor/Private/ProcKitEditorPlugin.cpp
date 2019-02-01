// Copyright  2019 Rasmus Egerö All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IProcKitEditorPlugin.h"
#include "LevelEditor.h"
#include "SlateBasics.h"
#include "ProcKitCommands.h"


class FProcKitEditorPlugin : public IProcKitEditorPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FExtender> ToolbarExtender;
	TSharedPtr<const FExtensionBase> Extension;

	void ProcKitButton_Clicked()
	{
		TSharedRef<SWindow> ProcKitWindow = SNew(SWindow)
			.Title(FText::FromString(TEXT("ProcKit Window")))
			.ClientSize(FVector2D(800, 400))
			.SupportsMaximize(false)
			.SupportsMinimize(false);

		IMainFrameModule& MainFrameModule =
			FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

		if (MainFrameModule.GetParentWindow().IsValid())
		{
			FSlateApplication::Get().AddWindowAsNativeChild(
				ProcKitWindow, MainFrameModule.GetParentWindow().ToSharedRef()
			);
		}
		else
		{
			FSlateApplication::Get().AddWindow(ProcKitWindow);
		}
	}

	void AddToolbarExtension(FToolBarBuilder &builder)
	{
		FSlateIcon IconBrush =
			FSlateIcon(FEditorStyle::GetStyleSetName(),
				"LevelEditor.ViewOptions",
				"LevelEditor.ViewOptions.Small");

		builder.AddToolBarButton(
			FProcKitCommands::Get().CommandButtonInfo,
			NAME_None,
			FText::FromString("ProcKit Button"),
			FText::FromString("Click to open ProcKit window."),
			IconBrush,
			NAME_None
		);
	}
};

IMPLEMENT_MODULE( FProcKitEditorPlugin, ProcKitEditor )



void FProcKitEditorPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	FProcKitCommands::Register();
	TSharedPtr<FUICommandList> CommandList =
		MakeShareable(new FUICommandList());
	
	CommandList->MapAction(FProcKitCommands::Get().CommandButtonInfo,
		FExecuteAction::CreateRaw(this,
			&FProcKitEditorPlugin::ProcKitButton_Clicked),
		FCanExecuteAction());
	
	ToolbarExtender = MakeShareable(new FExtender());
	Extension = ToolbarExtender
		->AddToolBarExtension("Compile", EExtensionHook::Before,
			CommandList, FToolBarExtensionDelegate::CreateRaw(this,
				&FProcKitEditorPlugin::AddToolbarExtension));
	
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>
		("LevelEditor");
	
	LevelEditorModule.GetToolBarExtensibilityManager()
		->AddExtender(ToolbarExtender);
}


void FProcKitEditorPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	ToolbarExtender->RemoveExtension(Extension.ToSharedRef());
	Extension.Reset();
	ToolbarExtender.Reset();
}

