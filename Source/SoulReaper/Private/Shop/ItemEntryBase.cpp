// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/ItemEntryBase.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "Shop/ShopTypes.h"


void UItemEntryBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ApplyIcon(nullptr);
	SlotButton->OnClicked.AddUniqueDynamic(this, &UItemEntryBase::HandleOnItemEntryClicked);
}

void UItemEntryBase::ClearEntry()
{
	RowName = NAME_None;

	ExpectedIcon.Reset();
	ApplyIcon(nullptr);
}

void UItemEntryBase::Init(FName InRowName, const FShopItemRow& InItemRow)
{
	RowName = InRowName;
	ExpectedIcon = InItemRow.Icon;

	// 이미 메모리에 올라와 있으면 기다릴 필요 없이 바로 적용
	if (UTexture2D* LoadedIcon = ExpectedIcon.Get())
	{
		ApplyIcon(LoadedIcon);
		return;
	}

	// 칸은 재사용되므로, 로드되는 동안 이전 아이템의 아이콘이 남아 보이지 않게 먼저 숨긴다
	ApplyIcon(nullptr);
	if (ExpectedIcon.IsNull())
		return;

	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		ExpectedIcon.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UItemEntryBase::HandleIconLoaded));
}

void UItemEntryBase::HandleOnItemEntryClicked()
{
	if (RowName.IsNone())
		return;

	OnItemEntryClicked.Broadcast(RowName);
}

void UItemEntryBase::HandleIconLoaded()
{
	// 콜백이 가져온 결과가 아니라 "지금 이 칸이 보여 줘야 하는 아이콘"을 적용한다.
	// 늦게 도착한 옛 요청의 콜백이 와도 잘못된 아이콘이 표시되지 않는다.
	ApplyIcon(ExpectedIcon.Get());
}

void UItemEntryBase::ApplyIcon(UTexture2D* InTexture)
{
	if (InTexture == nullptr)
	{
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	ItemImage->SetBrushFromTexture(InTexture);
	ItemImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}
