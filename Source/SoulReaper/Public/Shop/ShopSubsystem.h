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

class FJsonObject;
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
	void LoadWallet(FOnShopResponse OnDone = FOnShopResponse());
	void Checkout(FOnShopResponse OnDone);
	const FShopItemRow* FindItemRow(FName RowName) const;
	TArray<FName> GetAllItemRowNames() const;

	// 통신 결과에 해당하는 사용자용 에러 코드. 표에 없으면 기본 코드를 돌려준다
	FString GetErrorCode(ENetworkResponse InResult) const;

	// 늦게 구독한 쪽이 지금 상태를 읽을 수 있도록 연다. 값은 WalletState 가 Loaded 일 때만 믿을 수 있다.
	EWalletState GetWalletState() const { return WalletState; }
	int32 GetGold() const { return Gold; }
	const TMap<FName, int32>& GetInventory() const { return Inventory; }

private:
	UPROPERTY()
	TObjectPtr<UDataTable> ItemTable;

	UPROPERTY()
	TObjectPtr<UDataTable> NetworkErrorTable;

	int32 Gold = 0;
	TMap<FName, int32> Inventory;
	FString CachedAccountId;
	EWalletState WalletState = EWalletState::NotLoaded;

	FString GetAccountId();
	void AddGold(int32 InDelta);
	void AddItem(FName InRowName, int32 InDelta);
	void ApplyWalletJson(const TSharedPtr<FJsonObject>& Root);
	void HandleWalletResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnShopResponse OnDone);
	void HandleCheckoutResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnShopResponse OnDone);
};
