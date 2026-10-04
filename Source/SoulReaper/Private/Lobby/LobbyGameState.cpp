// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameState.h"

void ALobbyGameState::NotifyLobbyPlayersChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] 로비 변경 신호"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
	OnLobbyPlayersChanged.Broadcast();
}

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	NotifyLobbyPlayersChanged();
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	NotifyLobbyPlayersChanged();
}
