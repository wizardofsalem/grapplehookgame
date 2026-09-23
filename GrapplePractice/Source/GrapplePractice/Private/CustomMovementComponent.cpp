// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomMovementComponent.h"
#include "GrapplePracticeCharacter.h"

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{

	ECustomMovementMode Mode = static_cast<ECustomMovementMode>(CustomMovementMode);

	switch (Mode)
	{
	case CMOVE_Grapple:
		PhysGrapple(deltaTime, Iterations);
		break;
	}
}

void UCustomMovementComponent::PhysGrapple(float deltaTime, int32 Iterations) {
	if (deltaTime < MIN_TICK_TIME)
		return;

	if (!IsGrappleTaut() || !bGrappleAttached_)
		return;
	
	Super::CanAttemptJump();
	const FVector actorLocation = CharacterOwner->GetActorLocation();
	const FVector anchorDelta = grappleAnchor_ - actorLocation;
	const float distanceFromAnchor = anchorDelta.Size();
	const FVector grappleDirection = anchorDelta.GetSafeNormal();

	// Positive when moving away from the anchor (stretching the rope).
	const float RadialSpeedAwayFromAnchor = -FVector::DotProduct(Velocity, grappleDirection);

	if (RadialSpeedAwayFromAnchor >= RopeSnapSpeed_)
	{
		bGrappleAttached_ = false;
		if (AGrapplePracticeCharacter* MyCharacter = Cast<AGrapplePracticeCharacter>(CharacterOwner))
		{
			MyCharacter->DoGrappleEnd();
		}
		return;
	}
	const float Stretch = FMath::Max(0.0f, distanceFromAnchor - grappleLength_);
	const float RestoringForce = FMath::Max(0.0f,
		RopeSpringStiffness_ * Stretch + RopeSpringDamping_ * RadialSpeedAwayFromAnchor);

	FVector ropeAccel = grappleDirection * RestoringForce;
	FVector gravityAccel = FVector(0.0f, 0.0f, GetGravityZ());

	const FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
	const FVector AirResistanceAccel = -HorizontalVelocity * SwingAirResistance_;

	auto GrappleControlAcceleration = Acceleration * GrappleControl;

	FVector totalAccel = gravityAccel + ropeAccel + AirResistanceAccel + Acceleration;

	Velocity = Velocity + totalAccel * deltaTime;

	const FVector oldLocation = UpdatedComponent->GetComponentLocation();

	FHitResult Hit;
	SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);

	Velocity = (UpdatedComponent->GetComponentLocation() - oldLocation) / deltaTime;
}

void UCustomMovementComponent::AttachGrapple(FVector grappleAnchor, float grappleLength)
{
	grappleAnchor_ = grappleAnchor;
	grappleLength_ = grappleLength;
	bGrappleAttached_ = true;
	bGrappleFullyExtended_ = false;
}

bool UCustomMovementComponent::CanAttemptJump() const
{
	if (!(MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grapple)) {
		return Super::CanAttemptJump();
	}
	return !HasJumped_;
}

void UCustomMovementComponent::EndGrapple()
{
	bGrappleAttached_ = false;
	HasJumped_ = false;
	UE_LOG(LogTemp, Log, TEXT("Ending grapple %s"), bGrappleAttached_ ? TEXT("true") : TEXT("false"));
	if (MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grapple)
	{
		SetDefaultMovementMode();
	}
}

void UCustomMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (DeltaSeconds < MIN_TICK_TIME)
	{
		return;
	}

	if (!bGrappleAttached_ && InGrappleState())
	{
		SetDefaultMovementMode();
		return;
	}

	if (bGrappleAttached_ && !InGrappleState() && IsGrappleTaut())
	{
		SetMovementMode(MOVE_Custom, CMOVE_Grapple);
	}

	if (bGrappleAttached_ && InGrappleState() && !IsGrappleTaut()) {
		SetDefaultMovementMode();
	}

	FVector ropeDirection;
	double distanceFromAnchor;
	(grappleAnchor_ - UpdatedComponent->GetComponentLocation()).ToDirectionAndLength(ropeDirection, distanceFromAnchor);

	// Slack: rope isn't taut, leave whatever movement mode ran this frame alone.
	//if (distanceFromAnchor < grappleLength_)
	//{
	//	return;
	//}
	//else {
	//	const double outwardSpeed = FVector::DotProduct(Velocity, ropeDirection);
	//	if (outwardSpeed < 0.0)
	//	{
	//		Velocity -= ropeDirection * outwardSpeed;
	//	}

	//	FHitResult Hit;
	//	SafeMoveUpdatedComponent(ropeDirection * (distanceFromAnchor - grappleLength_), UpdatedComponent->GetComponentQuat(), true, Hit);
	//}
}

bool UCustomMovementComponent::IsGrappleTaut()
{
	const float Distance = (grappleAnchor_ - GetOwner()->GetActorLocation()).Size();

	if (!bGrappleFullyExtended_)
	{
		// Not latched yet - only a true full extension counts, no grace.
		bGrappleFullyExtended_ = Distance >= grappleLength_;
	}
	else
	{
		// Already latched - the grace zone keeps it taut through small dips (e.g. swing oscillation).
		const float TautThreshold = grappleLength_ * (1.0f - GrappleTautGracePercent_);
		bGrappleFullyExtended_ = Distance >= TautThreshold;
	}

	return bGrappleFullyExtended_;
}

bool UCustomMovementComponent::InGrappleState()
{
	return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grapple;
}