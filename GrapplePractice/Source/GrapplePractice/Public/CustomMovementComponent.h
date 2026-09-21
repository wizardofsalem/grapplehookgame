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
	void StartGrapple(FVector grappleAnchor, float grappleStrength, float grappleLength);
	void EndGrapple();

protected:
	/** Applies the rope constraint on top of whichever movement mode ran this frame. */
	void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

private:
	bool bGrappleAttached_ = false;
	FVector grappleAnchor_;
	float grappleStength_{};
	float grappleLength_{};
};
