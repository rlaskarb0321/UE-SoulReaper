// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/Shop/LobbyShop.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Common/UISubsystem.h"
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
	RequestWalletLoad();
}

void ULobbyShop::NativeDestruct()
{
	// 응답을 기다리는 중에 상점이 사라지면 HandleWalletLoaded 가 불리지 않는다. 로딩 UI 가 남지 않게 여기서 내린다.
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UUISubsystem* UISubsystem = GameInstance->GetSubsystem<UUISubsystem>())
		{
			UISubsystem->HideLoading();
		}
	}

	Super::NativeDestruct();
}

void ULobbyShop::RequestWalletLoad()
{
	// 응답이 올 때까지 로딩 UI 로 화면을 덮는다
	if (UUISubsystem* UISubsystem = GetGameInstance()->GetSubsystem<UUISubsystem>())
	{
		UISubsystem->ShowLoading();
	}

	FOnShopResponse OnDone;
	OnDone.BindUObject(this, &ULobbyShop::HandleWalletLoaded);
	ShopSubsystem->LoadWallet(OnDone);
}

void ULobbyShop::HandleWalletLoaded(const FShopResponse& Response)
{
	UUISubsystem* UISubsystem = GetGameInstance()->GetSubsystem<UUISubsystem>();
	if (UISubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ULobbyShop::HandleWalletLoaded] UISubsystem is nullptr"));
		return;
	}

	// 성공이든 실패든 응답이 왔으니 로딩 UI 는 먼저 내린다 (실패 팝업이 로딩 뒤에 가려지지 않게)
	UISubsystem->HideLoading();

	// 성공하면 지갑 값은 이미 OnRefreshGold/OnRefreshInventory 로 화면에 반영됐다
	if (Response.Result == EShopResult::Success)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[ULobbyShop::HandleWalletLoaded] Wallet load failed (result %d)"), static_cast<int32>(Response.Result));

	// 왼쪽(다시 시도): 지갑 조회를 다시 요청한다. 오른쪽(닫기): 할 일이 없으므로 넘기지 않는다.
	FOnPopupButtonClicked OnRetry;
	OnRetry.BindUObject(this, &ULobbyShop::RequestWalletLoad);
	UISubsystem->ShowPopup(WalletLoadFailedPopupRow.RowName, OnRetry);
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
