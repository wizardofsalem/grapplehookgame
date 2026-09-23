// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"


UENUM(BlueprintType)
enum ECustomMovementMode : uint8
{
	CMOVE_None UMETA(DisplayName = "None"),
	CMOVE_Grapple UMETA(DisplayName = "Grapple")
};

UCLASS()
class GRAPPLEPRACTICE_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	void PhysCustom(float deltaTime, int32 Iterations) override;
	void PhysGrapple(float deltaTime, int32 Iterations);
	void EndGrapple();
	void AttachGrapple(FVector grappleAnchor, float grappleLength);

	virtual bool CanAttemptJump() const;


	/** Grace zone below grappleLength_ (as a fraction, 0.05 = 5%) where the rope still counts as taut */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float GrappleTautGracePercent_ = 0.05f;

	/** Speed moving away from the anchor (stretching the rope) beyond which the rope snaps/detaches instead of rebounding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float RopeSnapSpeed_ = 3000.0f;

	/** Spring stiffness - how hard the rope pulls back per unit of stretch past grappleLength_ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float RopeSpringStiffness_ = 50.0f;

	/** Damping - resists radial velocity (either direction) so the bounce fizzles out instead of oscillating forever */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float RopeSpringDamping_ = 5.0f;

	/** Drag on horizontal (X/Y) velocity while swinging, so the swing gradually slows and stops instead of continuing forever */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float SwingAirResistance_ = 0.5f;

	/** How much the player input affects movement when grappling  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float GrappleControl = 1.0f;

protected:
	/** Applies the rope constraint on top of whichever movement mode ran this frame. */
	void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;


public:
	bool HasJumped_ = false;

private:

	bool IsGrappleTaut();
	bool InGrappleState();
	bool bGrappleAttached_ = false;

	/** Latch: true once the rope has reached true full extension. Only while latched does the
	 *  grace zone keep it counted as taut; dropping below the grace zone un-latches it, requiring
	 *  a fresh full-length touch (no grace) to count as taut again. */
	bool bGrappleFullyExtended_ = false;

	FVector grappleAnchor_;
	float grappleLength_{};
};
