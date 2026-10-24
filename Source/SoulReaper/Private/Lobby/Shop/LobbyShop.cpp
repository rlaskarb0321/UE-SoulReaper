// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/Shop/LobbyShop.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Shop/InventoryItem.h"
#include "Shop/ItemEntryBase.h"
#include "Shop/ShopSettings.h"
#include "Shop/ShopSubsystem.h"
#include "Shop/ShopTypes.h"

void ULobbyShop::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>();
	SetupItemEntrySlots();

	if (ShopSubsystem == nullptr)
		return;

	ShopItemRowNames = ShopSubsystem->GetAllItemRowNames();
	RefreshShopItemSlots();

	for (UItemEntryBase* InventoryItem : InventorySlots)
	{
		if (InventoryItem == nullptr)
			continue;

		InventoryItem->ClearEntry();
	}

	ShopSubsystem->OnRefreshGold.AddUObject(this, &ULobbyShop::HandleGoldChanged);
	ShopSubsystem->OnRefreshInventory.AddUObject(this, &ULobbyShop::HandleInventoryChanged);

	// 지갑 조회는 컨트롤러가 시작한다. 이 위젯이 만들어지기 전에 이미 끝났을 수 있으니, 불러온 값이 있으면 지금 한 번 그린다.
	if (ShopSubsystem->GetWalletState() == EWalletState::Loaded)
	{
		HandleGoldChanged(ShopSubsystem->GetGold());
		for (const TPair<FName, int32>& Pair : ShopSubsystem->GetInventory())
		{
			HandleInventoryChanged(Pair.Key, Pair.Value);
		}
	}
}

void ULobbyShop::NativeDestruct()
{
	// 위젯이 사라진 뒤에는 방송을 받을 필요가 없다
	if (ShopSubsystem != nullptr)
	{
		ShopSubsystem->OnRefreshGold.RemoveAll(this);
		ShopSubsystem->OnRefreshInventory.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void ULobbyShop::SetupItemEntrySlots()
{
	// Setup Inventory Slots
	InventorySlots.Empty();
	InventorySlots.SetNum(InventoryUniformGrid->GetChildrenCount());

	for (UWidget* Child : InventoryUniformGrid->GetAllChildren())
	{
		UItemEntryBase* Entry = Cast<UItemEntryBase>(Child);
		UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(Child->Slot);
		if (Entry == nullptr || GridSlot == nullptr)
			continue;

		const int32 Column = GridSlot->GetColumn();
		if (InventorySlots.IsValidIndex(Column) == false)
			continue;

		InventorySlots[Column] = Entry;
		Entry->OnItemEntryClicked.AddUObject(this, &ULobbyShop::HandleInventoryEntryClicked);
	}
	
	MaxInvenSize = GetDefault<UShopSettings>()->MaxBagKinds;
	CurrentInventory->SetText(FText::Format(
		FText::FromString(TEXT("{0} / {1}")),
		FText::AsNumber(CurrentInvenSize),
		FText::AsNumber(MaxInvenSize)));

	// Setup Shop Item Slots
	ShopItemSlots.Empty();
	ShopItemSlots.SetNum(ShopUniformGrid->GetChildrenCount());

	for (UWidget* Child : ShopUniformGrid->GetAllChildren())
	{
		UItemEntryBase* Entry = Cast<UItemEntryBase>(Child);
		UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(Child->Slot);
		if (Entry == nullptr || GridSlot == nullptr)
			continue;

		const int32 Column = GridSlot->GetColumn();
		if (ShopItemSlots.IsValidIndex(Column) == false)
			continue;

		ShopItemSlots[Column] = Entry;
		Entry->OnItemEntryClicked.AddUObject(this, &ULobbyShop::HandleShopItemEntryClicked);
	}
}

void ULobbyShop::HandleGoldChanged(int32 NewGold)
{
	GoldAmount->SetText(FText::AsNumber(NewGold));
}

void ULobbyShop::HandleInventoryChanged(FName RowName, int32 NewCount)
{
	UItemEntryBase* FoundItem = nullptr;

	for (UItemEntryBase* Entry : InventorySlots)
	{
		if (Entry == nullptr)
			continue;

		if (Entry->GetItemName() == RowName)
		{
			FoundItem = Entry;
			break;
		}
	}

	if (FoundItem != nullptr)
	{
		if (NewCount <= 0)
		{
			FoundItem->ClearEntry();
			CurrentInvenSize = FMath::Clamp(--CurrentInvenSize, 0, MaxInvenSize);
			CurrentInventory->SetText(FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			FText::AsNumber(CurrentInvenSize),
			FText::AsNumber(MaxInvenSize)));
			return;
		}

		UInventoryItem* InventoryItem = Cast<UInventoryItem>(FoundItem);
		if (InventoryItem == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[ULobbyShop::HandleInventoryChanged] '%s' slot is not UInventoryItem. Check the WBP parent class."), *RowName.ToString());
			return;
		}

		InventoryItem->SetAmount(NewCount);
		return;
	}

	if (NewCount <= 0)
		return;
	
	for (UItemEntryBase* Entry : InventorySlots)
	{
		if (Entry == nullptr)
			continue;
		if (Entry->GetItemName() != NAME_None)
			continue;
		
		UInventoryItem* InventoryItem = Cast<UInventoryItem>(Entry);
		if (InventoryItem == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[ULobbyShop::HandleInventoryChanged] '%s' slot is not UInventoryItem. Check the WBP parent class."), *RowName.ToString());
			return;
		}
		
		InventoryItem->Init(RowName, *ShopSubsystem->FindItemRow(RowName));
		InventoryItem->SetAmount(NewCount);
		CurrentInvenSize = FMath::Clamp(++CurrentInvenSize, 0, MaxInvenSize);
		CurrentInventory->SetText(FText::Format(
		FText::FromString(TEXT("{0} / {1}")),
		FText::AsNumber(CurrentInvenSize),
		FText::AsNumber(MaxInvenSize)));
		break;
	}
}

void ULobbyShop::HandleInventoryEntryClicked(FName RowName)
{
	if (ShopSubsystem == nullptr)
		return;

	EShopResult Result = ShopSubsystem->RequestRefund(RowName);
	if (Result == EShopResult::Success)
		return;

	switch (Result)
	{
	case EShopResult::InvalidItem:
		break;

	case EShopResult::NotOwned:
		break;

	default:
		break;
	}
}

void ULobbyShop::HandleShopItemEntryClicked(FName RowName)
{
	if (ShopSubsystem == nullptr)
		return;

	EShopResult Result = ShopSubsystem->RequestBuy(RowName);
	if (Result == EShopResult::Success)
		return;

	switch (Result)
	{
	case EShopResult::NotEnoughGold:
		PlayAnimation(NotEnoughGold);
		break;

	case EShopResult::BagFull:
		PlayAnimation(BagFull);
		break;

	case EShopResult::InvalidItem:
		break;

	default:
		break;
	}
}


void ULobbyShop::RefreshShopItemSlots()
{
	if (ShopSubsystem == nullptr)
		return;

	const int32 PerPage = ShopItemSlots.Num();
	if (PerPage <= 0)
		return;

	const int32 LastPage = FMath::Max(0, FMath::DivideAndRoundUp(ShopItemRowNames.Num(), PerPage) - 1);
	CurrentPage = FMath::Clamp(CurrentPage, 0, LastPage);

	for (int32 i = 0; i < PerPage; i++)
	{
		UItemEntryBase* FillTarget = ShopItemSlots[i];
		if (FillTarget == nullptr)
			continue;

		const int32 FindIndex = (PerPage * CurrentPage) + i;
		if (ShopItemRowNames.IsValidIndex(FindIndex))
		{
			const FShopItemRow* FindRow = ShopSubsystem->FindItemRow(ShopItemRowNames[FindIndex]);

			FillTarget->Init(ShopItemRowNames[FindIndex], *FindRow);
		}
		else
		{
			FillTarget->ClearEntry();
		}
	}
}
