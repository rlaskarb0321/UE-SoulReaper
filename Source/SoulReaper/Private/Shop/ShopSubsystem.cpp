// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/ShopSubsystem.h"

#include "Shop/ShopSettings.h"
#include "Shop/ShopTypes.h"

void UShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UShopSettings* Settings = GetDefault<UShopSettings>();
	
	Gold = Settings->StartingGold;
	ItemTable = Settings->ShopItemTable.LoadSynchronous();
	if (ItemTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UShopSubsystem::Initialize] ItemTable is nullptr"));
		return;
	}
}

EShopResult UShopSubsystem::RequestBuy(FName RowName)
{
	const UShopSettings* Settings = GetDefault<UShopSettings>();
	const FShopItemRow* RowItem = FindItemRow(RowName);
	
	if (RowItem == nullptr)
		return EShopResult::InvalidItem;
	
	if (Inventory.Contains(RowName) == false && Inventory.Num() >= Settings->MaxBagKinds)
		return EShopResult::BagFull;
	
	if (RowItem->Price > Gold)
		return EShopResult::NotEnoughGold;
	
	AddItem(RowName, 1);
	AddGold(-1 * RowItem->Price);
	return EShopResult::Success;
}

EShopResult UShopSubsystem::RequestRefund(FName RowName)
{
	const FShopItemRow* RowItem = FindItemRow(RowName);
	
	if (RowItem == nullptr)
		return EShopResult::InvalidItem;
	
	if (Inventory.Contains(RowName) == false)
		return EShopResult::NotOwned;
	
	AddItem(RowName, -1);
	AddGold(RowItem->Price);
	return EShopResult::Success;
}

const FShopItemRow* UShopSubsystem::FindItemRow(FName RowName) const
{
	if (ItemTable == nullptr)
		return nullptr;
	
	const FShopItemRow* Row = ItemTable->FindRow<FShopItemRow>(RowName, TEXT("UShopSubsystem::FindItemRow"), false);
	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UShopSubsystem::FindItemRow] Row '%s' does not exist in ItemTable"), *RowName.ToString());
		return nullptr;
	}
	
	return Row;
}

TArray<FName> UShopSubsystem::GetAllItemRowNames() const
{
	if (ItemTable == nullptr)
		return TArray<FName>();
	
	return ItemTable->GetRowNames();
}

void UShopSubsystem::AddGold(int32 InDelta)
{
	Gold += InDelta;
	OnRefreshGold.Broadcast(Gold);
}

void UShopSubsystem::AddItem(FName InRowName, int32 InDelta)
{
	int32& Count = Inventory.FindOrAdd(InRowName);
	Count += InDelta;

	const int32 NewCount = Count;      
	if (NewCount <= 0)
	{
		Inventory.Remove(InRowName);
	}

	OnRefreshInventory.Broadcast(InRowName, FMath::Max(NewCount, 0));
}
