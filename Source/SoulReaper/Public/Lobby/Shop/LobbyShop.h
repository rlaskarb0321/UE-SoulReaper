// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyShop.generated.h"

class UItemEntryBase;
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
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ShopUniformGrid;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> BagUniformGrid;
	
	virtual void NativeOnInitialized() override;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UItemEntryBase>> InventorySlots;
	
	void HandleGoldChanged(int32 NewGold);
	void HandleInventoryChanged(FName RowName, int32 NewCount);
	void CollectInventorySlots();
};
