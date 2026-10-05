// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerState.h"

#include "Lobby/LobbyGameState.h"

void ALobbyPlayerState::SetIsReady(bool bInIsReady)
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ALobbyPlayerState::SetIsReady] 클라이언트에서 호출됨 — 무시한다"));
		return;
	}

	if (bInIsReady == bIsReady)
		return;

	bIsReady = bInIsReady;

	if (ALobbyGameState* LobbyGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGS->RebuildLobbyPlayers();
	}
}
