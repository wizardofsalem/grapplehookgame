// Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleHud.h"
#include "Engine/Canvas.h"
#include "GrapplePracticePlayerController.h"


void AGrappleHud::DrawHUD() {
	auto grappleOwner = Cast<AGrapplePracticePlayerController>(PlayerOwner);

	if(!grappleOwner)
		return;

	FVector2D crosshairLocation = grappleOwner->getGrappleCrosshairLocation();

	Canvas->K2_DrawPolygon(nullptr, crosshairLocation, FVector2D(3,3), 12, FLinearColor(1.0, 0.0, 0.0, 1.0));
}