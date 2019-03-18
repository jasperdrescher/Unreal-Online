// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UnrealOnlineCppGameMode.h"
#include "UnrealOnlineCppHUD.h"
#include "UnrealOnlineCppCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUnrealOnlineCppGameMode::AUnrealOnlineCppGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AUnrealOnlineCppHUD::StaticClass();
}
