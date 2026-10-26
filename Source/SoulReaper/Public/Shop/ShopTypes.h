// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShopTypes.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FShopItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Shop")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Shop")
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Shop")
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Shop")
	int32 Price = 0;
};

UENUM(BlueprintType)
enum class EShopResult : uint8
{
	Success				UMETA(DisplayName = "Success"),
	NotEnoughGold		UMETA(DisplayName = "Not enough gold"),
	BagFull				UMETA(DisplayName = "Bag Full"),
	InvalidItem			UMETA(DisplayName = "Invalid item"),
	NotOwned			UMETA(DisplayName = "Not owned"),
	MAX					UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENetworkResponse : uint8
{
	Success				UMETA(DisplayName = "Success"),
	NetworkError		UMETA(DisplayName = "Network error"),
	Timeout				UMETA(DisplayName = "Timeout"),
	ServerError			UMETA(DisplayName = "Server error"),
	InvalidResponse		UMETA(DisplayName = "Invalid response"),
	MAX					UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FNetworkErrorRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Network")
	FString ErrorCode;
};

// 서버에서 지갑을 불러오는 일의 진행 상태. 늦게 구독한 쪽도 지금 값을 읽을 수 있게 서브시스템이 들고 있는다.
enum class EWalletState : uint8
{
	NotLoaded,
	Loading,
	Loaded,
	Failed
};

// 서버 요청(조회/결제)의 결과. 요청한 쪽에게만 돌려준다.
struct FShopResponse
{
	// 통신 결과. 요청이 정상적으로 오갔는지를 나타낸다
	ENetworkResponse NetworkResult = ENetworkResponse::Success;

	// 상점 규칙의 결과
	EShopResult Result = EShopResult::Success;
};

DECLARE_DELEGATE_OneParam(FOnShopResponse, const FShopResponse&);
