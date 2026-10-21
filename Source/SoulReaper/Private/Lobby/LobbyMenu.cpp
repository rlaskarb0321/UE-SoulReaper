// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenu.h"

#include "Common/UISubsystem.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/ELobbyRoute.h"
#include "Lobby/LobbyMenuPlayerController.h"
#include "Lobby/LobbyPlayerSlot.h"
#include "Lobby/LobbyTypes.h"
#include "Lobby/Shop/LobbyShop.h"
#include "Network/Request/RequestHandler.h"

void ULobbyMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	PlayerSlots.Reset();
	for (UWidget* Child : PlayerListBox->GetAllChildren())
	{
		ULobbyPlayerSlot* PlayerSlot = Cast<ULobbyPlayerSlot>(Child);
		if (PlayerSlot == nullptr)
			continue;
		
		PlayerSlots.Add(PlayerSlot);
	}
	
	PlayButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::HandleReadyClicked);
	HostButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::HandleHostButtonClicked);
	LeaveButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::HandleLeaveButtonClicked);
}

void ULobbyMenu::RefreshPlayers(const TArray<FLobbyPlayerEntry>& Entries)
{
	if (Entries.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Entries is Empty"));
		return;
	}
	if (Entries.Num() > PlayerSlots.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Entries Too many entries"));
		return;
	}
	
	const APlayerController* OwningController = GetOwningPlayer();
	const bool bIsHost = OwningController != nullptr && OwningController->HasAuthority();
	bool bLockShop = false;
	
	// Init PlayerSlot
	PlayButtonSwitcher->SetActiveWidgetIndex(static_cast<int32>(
		bIsHost ? EPlayButtonSwitcher::Host : EPlayButtonSwitcher::Client));
	
	for (int32 i = 0; i < PlayerSlots.Num(); i++)
	{
		if (Entries.IsValidIndex(i) == false)
		{
			PlayerSlots[i]->SetEmpty();
			continue;
		}

		PlayerSlots[i]->SetEntry(Entries[i]);
	}
	
	// Init Shop Lock State
	if (OwningController != nullptr && OwningController->PlayerState != nullptr)
	{
		int32 MyId = OwningController->PlayerState->GetPlayerId();
		for (const FLobbyPlayerEntry& Entry : Entries)
		{
			if (Entry.PlayerId != MyId)
				continue;
			
			bLockShop = (bIsHost == false && Entry.bIsReady);
			break;
		}
	}		
	
	LobbyShop->SetIsEnabled(bLockShop == false);
}

void ULobbyMenu::HandleReadyClicked()
{
	ALobbyMenuPlayerController* LobbyPC = GetOwningPlayer<ALobbyMenuPlayerController>();
	if (LobbyPC == nullptr)
		return;
	
	IRequestHandler* RequestHandler = Cast<IRequestHandler>(LobbyPC);
	if (RequestHandler == nullptr)
		return;
	
	RequestHandler->HandleRequest(static_cast<int32>(ELobbyRoute::ToggleReady));
}

void ULobbyMenu::HandleHostButtonClicked()
{
	const APlayerController* OwningController = GetOwningPlayer();
	if (OwningController == nullptr)
		return;
	
	const bool bIsHost = OwningController->HasAuthority();
	if (bIsHost == false)
		return;
	
	ALobbyMenuPlayerController* LobbyPC = GetOwningPlayer<ALobbyMenuPlayerController>();
	if (LobbyPC == nullptr)
		return;
	
	IRequestHandler* RequestHandler = Cast<IRequestHandler>(LobbyPC);
	if (RequestHandler == nullptr)
		return;
	
	RequestHandler->HandleRequest(static_cast<int32>(ELobbyRoute::EnterGame));
}

void ULobbyMenu::HandleLeaveButtonClicked()
{

	UUISubsystem* UISubsystem = GetGameInstance()->GetSubsystem<UUISubsystem>();
	if (UISubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ULobbyMenu::HandleLeaveButtonClicked] UISubsystem is nullptr"));
		return;
	}
	
	FOnPopupButtonClicked OnLeft;
	OnLeft.BindWeakLambda(this, [this]()
	{
		if (MainMenuLevel.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("[ULobbyMenu::HandleLeaveButtonClicked] MainMenu is Null"));
			return;
		}
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuLevel);
	});

	// 오른쪽(취소): 따로 할 일이 없으므로 넘기지 않는다. 팝업이 스스로 닫힌다.
	UISubsystem->ShowPopup(LeavePopupRow.RowName, OnLeft);
}
