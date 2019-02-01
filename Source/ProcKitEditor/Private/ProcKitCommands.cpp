// Copyright  2019 Rasmus Egerö All Rights Reserved.

#include "ProcKitCommands.h"
#include "IProcKitEditorPlugin.h"

void FProcKitCommands::RegisterCommands()
{
	// TODO: probably should not do this locally on every occassion?
#define LOCTEXT_NAMESPACE "ProcKit"
	
	UI_COMMAND(CommandButtonInfo, "ProcKit", "ProcKit toolbar command",
		EUserInterfaceActionType::Button, FInputGesture());

#undef LOCTEXT_NAMESPACE
}