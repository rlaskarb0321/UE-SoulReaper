// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Common/PopupSettings.h"
#include "Common/PopupTypes.h"
#include "Common/PopupWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/PlayerController.h"


void UUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();

	// 팝업 위젯 클래스를 미리 로드한다. 첫 ShowPopup 에서 디스크 로드로 프레임이 끊기는 것을 막기 위함이다.
	if (!Settings->PopupWidgetClass.IsNull())
	{
		PopupClassLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			Settings->PopupWidgetClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &UUISubsystem::HandlePopupClassLoaded));
	}

	PopupTable = Settings->PopupTable.LoadSynchronous();
	if (PopupTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::Initialize] Popup Table is nullptr"));
		return;
	}
}

void UUISubsystem::ShowPopup(FName InRowName, FOnPopupButtonClicked InOnLeft, FOnPopupButtonClicked InOnRight)
{
	// 1) 행을 찾는다
	const FPopupTypeRow* Row = FindPopupType(InRowName);
	if (Row == nullptr)
		return;

	// 2) 미리 로드해 둔 클래스를 쓴다. 아직 로드 중이라 비어 있을 때만 동기 로드로 대체한다
	UClass* PopupClass = CachedPopupClass;
	if (PopupClass == nullptr)
	{
		const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();
		PopupClass = Settings->PopupWidgetClass.LoadSynchronous();
		if (PopupClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowPopup] PopupWidgetClass is not set in Project Settings"));
			return;
		}
		CachedPopupClass = PopupClass;
	}

	// 3) 위젯을 만든다 (소유 플레이어가 필요하다)
	APlayerController* OwningPlayer = GetGameInstance()->GetFirstLocalPlayerController();
	if (OwningPlayer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowPopup] No local player controller"));
		return;
	}

	UPopupWidget* PopupWidget = CreateWidget<UPopupWidget>(OwningPlayer, PopupClass);
	if (PopupWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowPopup] Failed to create the popup widget"));
		return;
	}

	// 4) 내용과 버튼 동작을 넘기고 화면에 올린다
	PopupWidget->SetData(*Row, InOnLeft, InOnRight);
	PopupWidget->AddToViewport();
}

void UUISubsystem::HandlePopupClassLoaded()
{
	const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();
	CachedPopupClass = Settings->PopupWidgetClass.Get();
	if (CachedPopupClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::HandlePopupClassLoaded] Failed to preload the popup widget class"));
	}
}

const FPopupTypeRow* UUISubsystem::FindPopupType(FName InRowName) const
{
	if (PopupTable == nullptr)
		return nullptr;

	const FPopupTypeRow* Row = PopupTable->FindRow<FPopupTypeRow>(InRowName, TEXT("UUISubsystem::FindPopupType"), false);
	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UUISubsystem::FindPopupType] Row '%s' does not exist in PopupTable"), *InRowName.ToString());
		return nullptr;
	}

	return Row;
}
