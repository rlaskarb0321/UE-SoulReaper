// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenu.h"

#include "Lobby/LobbyTypes.h"

void ULobbyMenu::RefreshPlayers(const TArray<FLobbyPlayerEntry>& Entries)
{
	UE_LOG(LogTemp, Warning, TEXT("[Widget] 갱신 — %d 명"), Entries.Num());

	for (const FLobbyPlayerEntry& Entry : Entries)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Widget]   %s Ready=%s"),
			*Entry.PlayerName,
			Entry.bIsReady ? TEXT("O") : TEXT("X"));
	}
}
