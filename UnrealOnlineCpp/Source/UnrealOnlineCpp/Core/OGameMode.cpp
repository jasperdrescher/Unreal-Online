// Copyright (c) 2019 Jasper Drescher.

#include "OGameMode.h"
#include "../Gameplay/OPlayerHUD.h"
#include "../Gameplay/OPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AOGameMode::AOGameMode() : Super()
{
	// Set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/BP_FirstPersonCharacter"));
	if (PlayerPawnClassFinder.Succeeded())
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerPawnClassFinder failed!"));
	}

	// Use our custom HUD class
	HUDClass = AOPlayerHUD::StaticClass();
}
