// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ELobbyRoute.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ELobbyRoute : uint8
{
	None					UMETA(DisplayName = "None"),
	ToggleReady             UMETA(DisplayName = "ToggleReady"),
 	
	MAX						UMETA(Hidden)
};