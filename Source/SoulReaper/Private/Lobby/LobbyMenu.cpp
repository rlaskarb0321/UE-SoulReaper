// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenu.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Lobby/LobbyMenuPlayerController.h"
#include "Lobby/LobbyPlayerSlot.h"
#include "Lobby/LobbyTypes.h"

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
	if (ALobbyMenuPlayerController* LobbyPC = GetOwningPlayer<ALobbyMenuPlayerController>())
	{
		
	}
}
