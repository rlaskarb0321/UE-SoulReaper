// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyShop.generated.h"

class UButton;
class UItemEntryBase;
class UShopSubsystem;
class UUniformGridPanel;
class UTextBlock;
/**
 *
 */
UCLASS()
class SOULREAPER_API ULobbyShop : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> NotEnoughGold;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BagFull;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentInventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ShopUniformGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> InventoryUniformGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RightButton;

	int32 CurrentPage = 0;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TObjectPtr<UShopSubsystem> ShopSubsystem;

	UPROPERTY()
	TArray<TObjectPtr<UItemEntryBase>> ShopItemSlots;

	UPROPERTY()
	TArray<TObjectPtr<UItemEntryBase>> InventorySlots;

	TArray<FName> ShopItemRowNames;
	int32 CurrentInvenSize = 0;
	int32 MaxInvenSize = 0;

	void HandleGoldChanged(int32 NewGold);
	void HandleInventoryChanged(FName RowName, int32 NewCount);
	void HandleInventoryEntryClicked(FName RowName);
	void HandleShopItemEntryClicked(FName RowName);
	void SetupItemEntrySlots();
	void RefreshShopItemSlots();
};
