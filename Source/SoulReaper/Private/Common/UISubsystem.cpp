// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Common/PopupSettings.h"
#include "Common/PopupTypes.h"
#include "Common/PopupWidget.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"


void UUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();
	
	PopupTable = Settings->PopupTable.LoadSynchronous();
	if (PopupTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::Initialize] Popup Table is nullptr"));
		return;
	}
}

void UUISubsystem::ShowPopup(FName InRowName)
{
	// 1) 행을 찾는다
	const FPopupTypeRow* Row = FindPopupType(InRowName);
	if (Row == nullptr)
		return;

	// 2) 어떤 위젯을 만들지는 설정에서 읽는다
	const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();
	UClass* PopupClass = Settings->PopupWidgetClass.LoadSynchronous();
	if (PopupClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowPopup] PopupWidgetClass is not set in Project Settings"));
		return;
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

	// 4) 내용을 채우고 화면에 올린다
	PopupWidget->SetData(*Row);
	PopupWidget->AddToViewport();
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
