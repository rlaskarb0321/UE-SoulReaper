// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameState.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbyPlayers);
}

void ALobbyGameState::AddLobbyPlayer(const APlayerState* PlayerState)
{
	if (HasAuthority() == false || PlayerState == nullptr)
		return;
	
	FLobbyPlayerEntry& Entry = LobbyPlayers.AddDefaulted_GetRef();
	Entry.PlayerId = PlayerState->GetPlayerId();
	Entry.PlayerName = PlayerState->GetPlayerName();
	Entry.bIsReady = false;
	
	OnRep_LobbyPlayers();
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (HasAuthority() == false || PlayerState == nullptr)
		return;
	
	int32 PlayerId = PlayerState->GetPlayerId();
	int32* Index = PlayerIdToLobbyPlayerIndex.Find(PlayerId);
	
	if (Index == nullptr)
		return;
	
	LobbyPlayers.RemoveAt(*Index);
	OnRep_LobbyPlayers();
}

void ALobbyGameState::OnRep_LobbyPlayers()
{
	RebuildIndexCache();
	OnLobbyPlayersChanged.Broadcast();
}

void ALobbyGameState::RebuildIndexCache()
{
	PlayerIdToLobbyPlayerIndex.Reset();
	for (int32 i = 0; i < LobbyPlayers.Num(); i++)
	{
		PlayerIdToLobbyPlayerIndex.Add(LobbyPlayers[i].PlayerId, i);
	}
}
