// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/ShopSubsystem.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Lobby/Shop/LobbyShop.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Guid.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
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

FString UShopSubsystem::GetAccountId()
{
	if (CachedAccountId.IsEmpty() == false)
		return CachedAccountId;

	// 1) 저장된 ID 를 읽는다
	FString Id;
	GConfig->GetString(TEXT("SoulReaper"), TEXT("ShopAccountId"), Id, GGameUserSettingsIni);

	// 2) 없으면 처음 한 번만 만들어 저장한다
	if (Id.IsEmpty())
	{
		Id = FGuid::NewGuid().ToString(EGuidFormats::Digits);
		GConfig->SetString(TEXT("SoulReaper"), TEXT("ShopAccountId"), *Id, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}

	// 3) 에디터 PIE 에서 클라이언트를 여러 개 띄웠을 때 지갑이 겹치지 않게 구분한다
	const FWorldContext* Context = GetGameInstance()->GetWorldContext();
	if (Context != nullptr && Context->PIEInstance != INDEX_NONE)
	{
		Id += FString::Printf(TEXT("_pie%d"), Context->PIEInstance);
	}

	CachedAccountId = Id;
	return CachedAccountId;
}

void UShopSubsystem::LoadWallet()
{
	const UShopSettings* Settings = GetDefault<UShopSettings>();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s/wallet?accountId=%s"),
		*Settings->ServerBaseUrl, *FGenericPlatformHttp::UrlEncode(GetAccountId())));
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

	ApplyWalletJson(Root);
}

// 서버가 내려준 지갑({gold, inventory})으로 캐시를 교체하고 화면 갱신을 방송한다.
// 지갑 조회와 결제 응답이 같이 쓴다.
void UShopSubsystem::ApplyWalletJson(const TSharedPtr<FJsonObject>& Root)
{
	// 서버 값으로 Gold 교체 + 방송
	int32 NewGold = Gold;
	if (Root->TryGetNumberField(TEXT("gold"), NewGold))
	{
		Gold = NewGold;
		OnRefreshGold.Broadcast(Gold);
	}

	// inventory 가 없는 응답(예: 단순 거절)이면 가방은 건드리지 않는다
	const TSharedPtr<FJsonObject>* InventoryJson = nullptr;
	if (Root->TryGetObjectField(TEXT("inventory"), InventoryJson) == false)
		return;

	// 서버 값으로 Inventory 교체
	TMap<FName, int32> NewInventory;
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*InventoryJson)->Values)
	{
		NewInventory.Add(FName(*Pair.Key), static_cast<int32>(Pair.Value->AsNumber()));
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

// 로컬의 Gold 와 Inventory 를 그대로 서버에 저장한다. (검증은 이 서버의 역할이 아니다)
void UShopSubsystem::Checkout(FOnShopResponse OnDone)
{
	const UShopSettings* Settings = GetDefault<UShopSettings>();

	TSharedRef<FJsonObject> InventoryJson = MakeShared<FJsonObject>();
	for (const TPair<FName, int32>& Pair : Inventory)
	{
		InventoryJson->SetNumberField(Pair.Key.ToString(), Pair.Value);
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("accountId"), GetAccountId());
	Root->SetNumberField(TEXT("gold"), Gold);
	Root->SetObjectField(TEXT("inventory"), InventoryJson);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Root, Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s/checkout"), *Settings->ServerBaseUrl));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Body);
	Request->SetTimeout(Settings->RequestTimeoutSeconds);
	Request->OnProcessRequestComplete().BindUObject(this, &UShopSubsystem::HandleCheckoutResponse, OnDone);
	Request->ProcessRequest();
}

void UShopSubsystem::HandleCheckoutResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnShopResponse OnDone)
{
	FShopResponse Result;

	// (1) 연결 자체가 됐는가
	if (bWasSuccessful == false || Response.IsValid() == false)
	{
		const bool bTimedOut = Request.IsValid() && Request->GetFailureReason() == EHttpFailureReason::TimedOut;
		Result.Result = bTimedOut ? EShopResult::Timeout : EShopResult::NetworkError;
		OnDone.ExecuteIfBound(Result);
		return;
	}

	// (2) 상태 코드가 정상인가
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()) == false)
	{
		Result.Result = EShopResult::ServerError;
		OnDone.ExecuteIfBound(Result);
		return;
	}

	// (3) 본문(JSON)을 해석할 수 있는가
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool bParsed = FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid();

	Result.Result = bParsed ? EShopResult::Success : EShopResult::InvalidResponse;
	OnDone.ExecuteIfBound(Result);
}
