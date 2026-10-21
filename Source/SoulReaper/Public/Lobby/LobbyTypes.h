// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LobbyTypes.generated.h"

/**
 *
 */

// 로비의 진행 단계. Committing 동안 각 클라이언트가 구매 내역을 상점 서버에 결제한다.
UENUM(BlueprintType)
enum class ELobbyPhase : uint8
{
	Waiting			UMETA(DisplayName = "Waiting"),
	Committing		UMETA(DisplayName = "Committing"),
};

// 한 플레이어의 결제 진행 상태
UENUM(BlueprintType)
enum class ECheckoutState : uint8
{
	None			UMETA(DisplayName = "None"),
	Pending			UMETA(DisplayName = "Pending"),
	Done			UMETA(DisplayName = "Done"),
	Failed			UMETA(DisplayName = "Failed"),
};

USTRUCT(BlueprintType)
struct FLobbyPlayerEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	int32 PlayerId = INDEX_NONE;
	
	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	bool bIsReady = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	bool bIsHost = false;

	UPROPERTY(BlueprintReadOnly, Category = "SoulReaper|Lobby")
	ECheckoutState CheckoutState = ECheckoutState::None;
};
