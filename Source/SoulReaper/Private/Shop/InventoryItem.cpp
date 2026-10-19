// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/InventoryItem.h"

#include "Components/TextBlock.h"


void UInventoryItem::SetAmount(int32 NewAmount)
{
	AmountText->SetText(FText::AsNumber(NewAmount));
}

void UInventoryItem::ClearEntry()
{
	Super::ClearEntry();
	
	AmountText->SetText(FText::GetEmpty());
}

