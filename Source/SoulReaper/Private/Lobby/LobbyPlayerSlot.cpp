// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Lobby/LobbyTypes.h"

void ULobbyPlayerSlot::SetEntry(const FLobbyPlayerEntry& Entry)
{
	if (PlayerName == nullptr || ReadyIndicator == nullptr)
		return;
	
	PlayerName->SetText(FText::FromString(Entry.PlayerName));
	if (Entry.bIsHost)
	{
		ReadyIndicator->SetText(GetHostText());
		ApplySlotBackground(ELobbySlotState::Host);
	}
	else
	{
		ReadyIndicator->SetText(Entry.bIsReady ? GetReadyText() : GetUnreadyText());
		ApplySlotBackground(ELobbySlotState::Client);
	}
	ReadyIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void ULobbyPlayerSlot::SetEmpty()
{
	if (PlayerName == nullptr || ReadyIndicator == nullptr)
		return;
	
	ApplySlotBackground(ELobbySlotState::Empty);
	PlayerName->SetText(FText::GetEmpty());
	ReadyIndicator->SetVisibility(ESlateVisibility::Hidden);
}

void ULobbyPlayerSlot::ApplySlotBackground(ELobbySlotState NewState)
{
	if (SlotBackground == nullptr)
		return;
	
	const FSlateBrush* Brush = StateBrushes.Find(NewState);
	if (Brush == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ULobbyPlayerSlot::ApplySlotBackground] The brush for state %s has not been set."), *UEnum::GetValueAsString(NewState));
		return;
	}
	
	SlotBackground->SetBrush(*Brush);
}
