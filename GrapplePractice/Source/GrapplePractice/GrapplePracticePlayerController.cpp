// Copyright Epic Games, Inc. All Rights Reserved.


#include "GrapplePracticePlayerController.h"
#include "GrapplePracticeCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "GrapplePractice.h"
#include "Widgets/Input/SVirtualJoystick.h"

FVector2D AGrapplePracticePlayerController::getGrappleCrosshairLocation() const
{
	auto character = Cast<AGrapplePracticeCharacter>(GetCharacter());


	if(!character)
		return FVector2D(0, 0);

		auto grappleStartHeight = character->GetGrappleStartHeight();
		auto grapplePitch = character->GetGrapplePitch();
		auto grappleDistance = character->GetGrappleDistance();

		FVector end = (PlayerCameraManager->GetCameraRotation() + FRotator(grapplePitch, 0.0f, 0.0f)).Vector() * grappleDistance;
		FVector start = PlayerCameraManager->GetCameraLocation() + FVector3d(0, 0, grappleStartHeight);

		FVector2D screenLocation;

		ProjectWorldLocationToScreen(start + end, screenLocation);
		return screenLocation;
}

void AGrapplePracticePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogGrapplePractice, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AGrapplePracticePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AGrapplePracticePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
