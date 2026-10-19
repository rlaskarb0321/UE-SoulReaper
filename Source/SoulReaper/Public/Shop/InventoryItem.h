// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shop/ItemEntryBase.h"
#include "InventoryItem.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class SOULREAPER_API UInventoryItem : public UItemEntryBase
{
	GENERATED_BODY()
	
public:
	void SetAmount(int32 NewAmount);
	virtual void ClearEntry() override;
	
protected:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmountText;
};
