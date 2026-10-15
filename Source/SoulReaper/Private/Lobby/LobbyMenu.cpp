// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenu.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Lobby/ELobbyRoute.h"
#include "Lobby/LobbyMenuPlayerController.h"
#include "Lobby/LobbyPlayerSlot.h"
#include "Lobby/LobbyTypes.h"
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
