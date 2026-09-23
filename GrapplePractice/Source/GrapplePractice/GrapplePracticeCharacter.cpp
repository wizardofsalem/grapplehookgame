// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrapplePracticeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GrapplePractice.h"
#include "Kismet/GameplayStatics.h"
#include <CustomMovementComponent.h>

AGrapplePracticeCharacter::AGrapplePracticeCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SetupGrappleCable();
}

void AGrapplePracticeCharacter::SetupGrappleCable()
{
	GrappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("Rope"));
	GrappleCable->SetupAttachment(GetMesh());
	GrappleCable->bAutoActivate = true;
	GrappleCable->NumSegments = 250;
	GrappleCable->NumSides = 16;
	GrappleCable->SolverIterations = 16;
	GrappleCable->CableGravityScale = 2.5f;
	GrappleCable->CableWidth = 3.0f;
	GrappleCable->bAttachStart = true;
	GrappleCable->bEnableCollision = true;

	GrappleAnchorPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GrappleAnchorPoint"));

	GrappleCable->bAttachEnd = true;
	GrappleCable->EndLocation = FVector::ZeroVector;
	GrappleCable->CableLength = GrappleDistance;
}

void AGrapplePracticeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Started, this, &AGrapplePracticeCharacter::DoGrappleStart);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Completed, this, &AGrapplePracticeCharacter::DoGrappleEnd);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGrapplePracticeCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AGrapplePracticeCharacter::Look);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGrapplePracticeCharacter::Look);

		EnhancedInputComponent->BindAction(SaveLocationAction, ETriggerEvent::Started, this, &AGrapplePracticeCharacter::DoSaveLocation);

		EnhancedInputComponent->BindAction(LoadLocationAction, ETriggerEvent::Started, this, &AGrapplePracticeCharacter::DoLoadLocation);

		EnhancedInputComponent->BindAction(BurstSpeedAction, ETriggerEvent::Started, this, &AGrapplePracticeCharacter::DoBurstSpeed);
	}
	else
	{
		UE_LOG(LogGrapplePractice, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGrapplePracticeCharacter::BeginPlay()
{
	Super::BeginPlay();

	GrappleCable->SetAttachEndToComponent(GrappleAnchorPoint, NAME_None);

	FAttachmentTransformRules AttachmentRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		false
	);
	GrappleCable->AttachToComponent(GetMesh(), AttachmentRules, FName("hand_l"));

	AddTickPrerequisiteComponent(GetMesh());
	GrappleCable->AddTickPrerequisiteActor(this);
	GrappleCable->SetVisibility(false);
}

void AGrapplePracticeCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (IsGrappling_)
	{
		const float Distance = FVector::Dist(GrappleCable->GetComponentLocation(), GrappleAnchorPoint->GetComponentLocation());
		GrappleCable->CableLength = FMath::Min(Distance * CableSlackMultiplier, FiredCableDistance);
	}
}

void AGrapplePracticeCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	DoMove(MovementVector.X, MovementVector.Y);
}

void AGrapplePracticeCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AGrapplePracticeCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AGrapplePracticeCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AGrapplePracticeCharacter::DoJumpStart()
{
	Jump();
}

void AGrapplePracticeCharacter::DoJumpEnd()
{
	StopJumping();
}

void AGrapplePracticeCharacter::DoGrappleStart()
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector traceStart = CameraLocation + FVector(0.0f, 0.0f, GrappleStartHeight);

	FVector LookDirection = (PC->PlayerCameraManager->GetCameraRotation() + FRotator(GrapplePitch, 0.0f, 0.0f)).Vector();
	
	FHitResult Hit;
	FCollisionQueryParams queryParams(FName(TEXT("CameraTrace")), true, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, traceStart, traceStart + LookDirection * GrappleDistance, ECC_Visibility, queryParams)) {
		AnchorLocation_ = Hit.Location;
		IsGrappling_ = true;
		GrappleAnchorPoint->SetWorldLocation(Hit.Location);
		FiredCableDistance = FVector::Dist(GrappleCable->GetComponentLocation(), Hit.Location);
		GrappleCable->SetVisibility(true);
		GetCharacterMovement<UCustomMovementComponent>()->AttachGrapple(Hit.Location, GrappleLaunchStrength, GrappleDistance);
	}
}

void AGrapplePracticeCharacter::DoGrappleEnd()
{
	IsGrappling_ = false;
	GrappleCable->SetVisibility(false);
	GetCharacterMovement<UCustomMovementComponent>()->EndGrapple();
}

void AGrapplePracticeCharacter::DoSaveLocation()
{
	if (GetCharacterMovement()->IsMovingOnGround()) {
	SavedLocation_ = GetActorLocation();
	}
}

void AGrapplePracticeCharacter::DoLoadLocation()
{
	if (SavedLocation_.IsSet())
	{
		SetActorLocation(SavedLocation_.GetValue());
	}
}

void AGrapplePracticeCharacter::DoBurstSpeed()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastBurstSpeedTime_ < BurstSpeedCooldown_)
	{
		return;
	}
	LastBurstSpeedTime_ = CurrentTime;

	FVector Forward = GetActorForwardVector();
	FVector HorizontalDirection = FVector(Forward.X, Forward.Y, 0.0f).GetSafeNormal();

	FVector FinalLaunchVelocity = HorizontalDirection * BurstSpeedForward_ + FVector(0.0f, 0.0f, BurstSpeedUp_);

	LaunchCharacter(FinalLaunchVelocity, true, false);
	OnBurstSpeedFired();
}
