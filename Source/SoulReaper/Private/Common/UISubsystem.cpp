// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Common/LoadingUI.h"
#include "Common/PopupSettings.h"
#include "Common/PopupTypes.h"
#include "Common/PopupWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


namespace
{
	constexpr int32 LoadingZOrder = 100;
}

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

void UUISubsystem::ShowPopup(FName InRowName, FOnPopupButtonClicked InOnLeft, FOnPopupButtonClicked InOnRight,
	const FFormatNamedArguments& InArgs)
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

	// 4) 호출자가 넘긴 동작이 없으면 행에 지정된 동작을 쓰고, 내용과 함께 넘겨서 화면에 올린다
	if (InOnLeft.IsBound() == false)
	{
		InOnLeft = MakeActionDelegate(Row->LeftAction);
	}
	if (InOnRight.IsBound() == false)
	{
		InOnRight = MakeActionDelegate(Row->RightAction);
	}

	PopupWidget->SetData(*Row, InOnLeft, InOnRight, InArgs);
	PopupWidget->AddToViewport();
}

void UUISubsystem::ShowLoading()
{
	if (LoadingWidget != nullptr && LoadingWidget->IsInViewport())
		return;

	const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();
	UClass* LoadingClass = Settings->LoadingWidgetClass.LoadSynchronous();
	if (LoadingClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowLoading] LoadingWidgetClass is not set in Project Settings"));
		return;
	}

	APlayerController* OwningPlayer = GetGameInstance()->GetFirstLocalPlayerController();
	if (OwningPlayer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowLoading] No local player controller"));
		return;
	}

	LoadingWidget = CreateWidget<ULoadingUI>(OwningPlayer, LoadingClass);
	if (LoadingWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::ShowLoading] Failed to create the loading widget"));
		return;
	}

	// 위젯은 CreateWidget 시점(NativeOnInitialized)에 로딩을 요청하고 나중에 화면에 올라가는 경우가 있다.
	// 올라가는 순서와 상관없이 항상 덮도록 ZOrder 를 높게 준다.
	LoadingWidget->AddToViewport(LoadingZOrder);
}

void UUISubsystem::HideLoading()
{
	if (LoadingWidget == nullptr)
		return;

	LoadingWidget->RemoveFromParent();
	LoadingWidget = nullptr;
}

FOnPopupButtonClicked UUISubsystem::MakeActionDelegate(EPopupAction Action)
{
	FOnPopupButtonClicked Delegate;

	switch (Action)
	{
	case EPopupAction::GoToMainMenu:
		Delegate.BindUObject(this, &UUISubsystem::GoToMainMenu);
		break;
	case EPopupAction::QuitGame:
		Delegate.BindUObject(this, &UUISubsystem::QuitGame);
		break;
	default:
		break;
	}

	return Delegate;
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

void UUISubsystem::GoToMainMenu()
{
	const UPopupWidgetSettings* Settings = GetDefault<UPopupWidgetSettings>();
	if (Settings->MainMenuLevel.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[UUISubsystem::GoToMainMenu] MainMenuLevel is not set in Project Settings"));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Settings->MainMenuLevel);
}

void UUISubsystem::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetGameInstance()->GetFirstLocalPlayerController(), EQuitPreference::Quit, false);
}
