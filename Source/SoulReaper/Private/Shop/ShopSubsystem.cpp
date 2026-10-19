// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/ShopSubsystem.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
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

void UShopSubsystem::LoadWallet()
{
	const UShopSettings* Settings = GetDefault<UShopSettings>();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s/wallet?playerId=player1"), *Settings->ServerBaseUrl));    
	Request->SetVerb(TEXT("GET"));
	Request->SetTimeout(Settings->RequestTimeoutSeconds);
	Request->OnProcessRequestComplete().BindUObject(this, &UShopSubsystem::HandleWalletResponse);
	Request->ProcessRequest();
}

void UShopSubsystem::HandleWalletResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// (1) 연결 자체가 됐는가
	if (bWasSuccessful == false || Response.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UShopSubsystem::HandleWalletResponse] Connection failed or timed out"));
		return;
	}

	// (2) 상태 코드가 정상인가
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UShopSubsystem::HandleWalletResponse] Status %d"), Response->GetResponseCode());
		return;
	}
	
	// (3) 본문(JSON)을 해석할 수 있는가
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, Root) == false || Root.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UShopSubsystem::HandleWalletResponse] Invalid JSON"));
		return;
	}

	// 서버 값으로 Gold 교체 + 방송
	int32 NewGold = Gold;
	Root->TryGetNumberField(TEXT("gold"), NewGold);
	Gold = NewGold;
	OnRefreshGold.Broadcast(Gold);

	// 서버 값으로 Inventory 교체
	TMap<FName, int32> NewInventory;
	const TSharedPtr<FJsonObject>* InventoryJson = nullptr;
	if (Root->TryGetObjectField(TEXT("inventory"), InventoryJson))
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*InventoryJson)->Values)
		{
			NewInventory.Add(FName(*Pair.Key), static_cast<int32>(Pair.Value->AsNumber()));
		}
	}

	// 예전 캐시에는 있었는데 서버 값에는 없는 아이템은 0개로 방송 (화면에서 지우기)
	for (const TPair<FName, int32>& Old : Inventory)
	{
		if (NewInventory.Contains(Old.Key) == false)
			OnRefreshInventory.Broadcast(Old.Key, 0);
	}

	Inventory = NewInventory;
	for (const TPair<FName, int32>& Pair : Inventory)
	{
		OnRefreshInventory.Broadcast(Pair.Key, Pair.Value);
	}
}
