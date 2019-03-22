// Copyright (c) 2019 Jasper Drescher.

#include "OPlayerHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"

AOPlayerHUD::AOPlayerHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPersonCPP/Textures/FirstPersonCrosshair"));
	if (CrosshairTexObj.Succeeded())
	{
		CrosshairTex = CrosshairTexObj.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CrosshairTexObj failed!"));
	}
}

void AOPlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// Find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// Offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition((Center.X), (Center.Y + 20.0f));

	// Draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
}
