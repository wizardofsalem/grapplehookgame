// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomMovementComponent.h"

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

	FVector actorLocation = GetOwner()->GetActorLocation();

	FVector anchorDelta = grappleAnchor_ - actorLocation;
	auto distanceFromAnchor = (anchorDelta).Size();
	FVector grappleDirection = anchorDelta.GetSafeNormal();

	FVector ropeAccel = grappleDirection * grappleStength_;
	if (distanceFromAnchor < grappleLength_) {
		ropeAccel = FVector::ZeroVector;
	}

	FVector acceleration = Acceleration;

	FVector gravityAccel = FVector(0.0f, 0.0f, GetGravityZ());

	FVector totalAccel = acceleration + gravityAccel + ropeAccel;
	Velocity = Velocity + totalAccel * deltaTime;

	const FVector oldLocation = UpdatedComponent->GetComponentLocation();

	FHitResult Hit;
	SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);

	// Recompute velocity from the distance actually travelled, so anything the sweep
	// clamped (walls, ledges) is reflected instead of accumulating unbounded.
	Velocity = (UpdatedComponent->GetComponentLocation() - oldLocation) / deltaTime;
}

void UCustomMovementComponent::StartGrapple(FVector grappleAnchor, float grappleStrength, float grappleLength)
{
	grappleAnchor_ = grappleAnchor;
	grappleStength_ = grappleStrength;
	grappleLength_ = grappleLength;
	bGrappleAttached_ = true;
}

void UCustomMovementComponent::EndGrapple()
{
	bGrappleAttached_ = false;

	// If we were swinging, hand control back to the engine.
	if (MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grapple)
	{
		SetDefaultMovementMode();
	}
}

void UCustomMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (!bGrappleAttached_ || DeltaSeconds < MIN_TICK_TIME)
	{
		return;
	}

	FVector ropeDirection;
	double distanceFromAnchor;
	(grappleAnchor_ - UpdatedComponent->GetComponentLocation()).ToDirectionAndLength(ropeDirection, distanceFromAnchor);

	// Slack rope does nothing - whatever walking/falling just did stands.
	if (distanceFromAnchor <= grappleLength_)
	{
		return;
	}

	// Taut: cancel any velocity carrying us further from the anchor.
	// ropeDirection points at the anchor, so moving away is a negative dot product.
	const double outwardSpeed = FVector::DotProduct(Velocity, ropeDirection);
	if (outwardSpeed < 0.0)
	{
		Velocity -= ropeDirection * outwardSpeed;
	}

	// Pull the capsule back onto the rope sphere, swept so it cannot clip through geometry.
	FHitResult Hit;
	SafeMoveUpdatedComponent(ropeDirection * (distanceFromAnchor - grappleLength_), UpdatedComponent->GetComponentQuat(), true, Hit);
}
