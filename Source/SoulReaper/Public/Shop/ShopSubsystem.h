// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShopTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "ShopSubsystem.generated.h"

/**
 * 
 */

struct FShopItemRow;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRefreshGold, int32 NewGold);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRefreshInventory, FName RowName, int32 NewCount);

UCLASS()
class SOULREAPER_API UShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	FOnRefreshGold OnRefreshGold;
	FOnRefreshInventory OnRefreshInventory;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	EShopResult RequestBuy(FName RowName);
	EShopResult RequestRefund(FName RowName);
	void LoadWallet();
	const FShopItemRow* FindItemRow(FName RowName) const;
	TArray<FName> GetAllItemRowNames() const;
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> ItemTable;
	
	int32 Gold = 0;
	TMap<FName, int32> Inventory;
	
	void AddGold(int32 InDelta);
	void AddItem(FName InRowName, int32 InDelta);
	void HandleWalletResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
