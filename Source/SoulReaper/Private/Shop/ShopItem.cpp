// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/ShopItem.h"

#include "Components/TextBlock.h"
#include "Shop/ShopTypes.h"

void UShopItem::Init(FName InRowName, const FShopItemRow& InItemRow)
{
	Super::Init(InRowName, InItemRow);
	
	PriceText->SetText(FText::FromString(FString::Printf(TEXT("%d G"), InItemRow.Price)));
}

void UShopItem::ClearEntry()
{
	Super::ClearEntry();
	
	PriceText->SetText(FText::GetEmpty());
}
