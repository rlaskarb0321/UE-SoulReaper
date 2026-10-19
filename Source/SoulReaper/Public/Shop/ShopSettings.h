// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ShopSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Shop"))
class SOULREAPER_API UShopSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Bag", meta = (ClampMin = "1"))
	int32 MaxBagKinds = 3;

	UPROPERTY(Config, EditAnywhere, Category = "Gold", meta = (ClampMin = "0"))
	int32 StartingGold = 1000;

	UPROPERTY(Config, EditAnywhere, Category = "Item")
	TSoftObjectPtr<UDataTable> ShopItemTable;

	UPROPERTY(Config, EditAnywhere, Category = "Server")
	FString ServerBaseUrl = TEXT("http://localhost:5080");

	UPROPERTY(Config, EditAnywhere, Category = "Server", meta = (ClampMin = "0.1"))
	float RequestTimeoutSeconds = 5.f;
};
