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