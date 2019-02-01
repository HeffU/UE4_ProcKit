// Copyright  2019 Rasmus Egerö All Rights Reserved.

#pragma once

#include "Commands.h"
#include "EditorStyleSet.h"

/**
 *
 */
class FProcKitCommands : public TCommands<FProcKitCommands>
{
public:
	FProcKitCommands()
		:TCommands<FProcKitCommands>(
			FName(TEXT("ProcKit")),
			FText::FromString("ProcKit Commands"),
			NAME_None,
			FEditorStyle::GetStyleSetName())
	{
	};

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> CommandButtonInfo;
};