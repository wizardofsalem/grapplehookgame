// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Misc/Optional.h"
#include "CableComponent.h"
#include "GrapplePracticeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AGrapplePracticeCharacter : public ACharacter
{
	GENERATED_BODY()

	// --- Components ---

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	UCableComponent* GrappleCable;

	/** Movable, unattached marker the cable's end is pinned to - repositioned to the trace hit each time the grapple fires */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	USceneComponent* GrappleAnchorPoint;

	/** Creates and configures GrappleCable/GrappleAnchorPoint - called from the constructor */
	void SetupGrappleCable();

protected:

	// --- Input Actions ---

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* GrappleAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SaveLocationAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LoadLocationAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BurstSpeedAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	// --- Grapple tunables/state ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float GrappleLaunchStrength=2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float GrappleStartHeight=1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float GrappleDistance = 1000.0f;

	/** How much extra slack the cable gets over the taut distance, as a multiplier (1.1 = 10% loose) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float CableSlackMultiplier = 1.1f;

	/** Actual hand-to-anchor distance captured at fire time - the real max reach, not GrappleDistance (which is measured from the camera) */
	float FiredCableDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float GrapplePitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grapple")
	FVector AnchorLocation_ = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grapple")
	bool IsGrappling_ = false;

	// --- Other gameplay state ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float BurstSpeed_ = 2000.0f;

	TOptional<FVector> SavedLocation_ = TOptional<FVector>();

	// --- Engine lifecycle / internal input handling ---

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Constructor */
	AGrapplePracticeCharacter(const FObjectInitializer& ObjectInitializer);

	// --- Public API (called from Blueprint/input bindings) ---

	void FireTetherLine();

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoGrappleStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoGrappleEnd();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoSaveLocation();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLoadLocation();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoBurstSpeed();

	// --- Getters ---

	float GetGrappleStartHeight() const { return GrappleStartHeight; }
	float GetGrapplePitch() const { return GrapplePitch; }
	float GetGrappleDistance() const { return GrappleDistance; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
