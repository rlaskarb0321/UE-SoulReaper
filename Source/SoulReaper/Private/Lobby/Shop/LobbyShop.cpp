// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/Shop/LobbyShop.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Shop/ItemEntryBase.h"
#include "Shop/ShopSubsystem.h"

void ULobbyShop::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	CollectInventorySlots();
	
	UShopSubsystem* ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>();
	if (ShopSubsystem == nullptr)
		return;
	
	HandleGoldChanged(ShopSubsystem->GetGold());	
	for (const TPair<FName, int32>& Pair : ShopSubsystem->GetInventory())
	{
		HandleInventoryChanged(Pair.Key, Pair.Value);
	}
	
	ShopSubsystem->OnGoldChanged.AddUObject(this, &ULobbyShop::HandleGoldChanged);
	ShopSubsystem->OnInventoryChanged.AddUObject(this, &ULobbyShop::HandleInventoryChanged);
}

void ULobbyShop::HandleGoldChanged(int32 NewGold)
{
	GoldAmount->SetText(FText::AsNumber(NewGold));
}

void ULobbyShop::HandleInventoryChanged(FName RowName, int32 NewCount)
{
	
}

void ULobbyShop::CollectInventorySlots()
{
	InventorySlots.Empty();
	InventorySlots.SetNum(BagUniformGrid->GetChildrenCount());
	
	for (UWidget* Child : BagUniformGrid->GetAllChildren())
	{
		UItemEntryBase* Entry = Cast<UItemEntryBase>(Child);
		UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(Child->Slot);
		if (Entry == nullptr || GridSlot == nullptr)
			continue;

		const int32 Column = GridSlot->GetColumn();
		if (InventorySlots.IsValidIndex(Column) == false)
			continue;

		InventorySlots[Column] = Entry;
	}
}
