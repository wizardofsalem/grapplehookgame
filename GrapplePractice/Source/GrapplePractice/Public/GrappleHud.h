// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GrappleHud.generated.h"

/**
 * 
 */
UCLASS()
class GRAPPLEPRACTICE_API AGrappleHud : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	
};
