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
	
	NetworkError		UMETA(DisplayName = "Network error"),
	Timeout				UMETA(DisplayName = "Timeout"),
	ServerError			UMETA(DisplayName = "Server error"),
	InvalidResponse		UMETA(DisplayName = "Invalid response"),
	MAX					UMETA(Hidden)
};

// 서버 요청(조회/결제)의 결과. 요청한 쪽에게만 돌려준다.
struct FShopResponse
{
	EShopResult Result = EShopResult::Success;
};

DECLARE_DELEGATE_OneParam(FOnShopResponse, const FShopResponse&);
