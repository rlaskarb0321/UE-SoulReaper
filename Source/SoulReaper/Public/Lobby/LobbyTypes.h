// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LobbyTypes.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FLobbyPlayerEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	bool bIsReady = false;
};
