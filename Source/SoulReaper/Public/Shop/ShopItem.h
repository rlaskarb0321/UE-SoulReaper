// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shop/ItemEntryBase.h"
#include "ShopItem.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class SOULREAPER_API UShopItem : public UItemEntryBase
{
	GENERATED_BODY()

public:
	virtual void Init(FName InRowName, const FShopItemRow& InItemRow) override;
	virtual void ClearEntry() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PriceText;
	
};
