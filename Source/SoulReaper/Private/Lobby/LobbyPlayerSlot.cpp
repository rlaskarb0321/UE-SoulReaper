// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerSlot.h"

#include "Components/TextBlock.h"
#include "Lobby/LobbyTypes.h"

void ULobbyPlayerSlot::SetEntry(const FLobbyPlayerEntry& Entry)
{
	if (PlayerName == nullptr || ReadyIndicator == nullptr)
		return;
	
	PlayerName->SetText(FText::FromString(Entry.PlayerName));
	ReadyIndicator->SetText(Entry.bIsReady ? GetReadyText() : GetUnreadyText());
	ReadyIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void ULobbyPlayerSlot::SetEmpty()
{
	if (PlayerName == nullptr || ReadyIndicator == nullptr)
		return;
	
	PlayerName->SetText(FText::GetEmpty());
	ReadyIndicator->SetVisibility(ESlateVisibility::Hidden);
}
