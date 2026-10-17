// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShopSubsystem.generated.h"

/**
 * 
 */

struct FShopItemRow;

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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32 NewGold);

UCLASS()
class SOULREAPER_API UShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	FOnGoldChanged OnGoldChanged;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	EShopResult RequestBuy(FName RowName);
	EShopResult RequestRefund(FName RowName);
	int32 GetGold() const { return Gold; }
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> ItemTable;
	
	int32 Gold = 0;
	TMap<FName, int32> Inventory;
	
	const FShopItemRow* FindItemRow(FName RowName) const;
	void AddGold(int32 InDelta);
};
