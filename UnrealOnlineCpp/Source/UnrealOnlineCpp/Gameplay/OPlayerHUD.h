// Copyright (c) 2019 Jasper Drescher.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OPlayerHUD.generated.h"

UCLASS()
class UNREALONLINECPP_API AOPlayerHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AOPlayerHUD();

	// Primary draw call for the HUD 
	virtual void DrawHUD() override;

private:
	// Crosshair asset pointer
	class UTexture2D* CrosshairTex;
};
