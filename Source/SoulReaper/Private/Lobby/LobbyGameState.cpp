// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameState.h"

#include "GameFramework/PlayerState.h"
#include "Lobby/ELobbyRoute.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbyPlayers);
}

void ALobbyGameState::HandleLobbyRequest(int32 InRequesterId, int32 InPayload)
{
	if (HasAuthority() == false)
		return;
	if (InPayload <= static_cast<int32>(ELobbyRoute::None) || InPayload >= static_cast<int32>(ELobbyRoute::MAX))
		return;
	
	const int32* Index = PlayerIdToLobbyPlayerIndex.Find(InRequesterId);
	if (Index == nullptr || LobbyPlayers.IsValidIndex(*Index) == false)
		return;

	FLobbyPlayerEntry& Entry = LobbyPlayers[*Index];
	ELobbyRoute LobbyRoute = static_cast<ELobbyRoute>(InPayload);

	switch (LobbyRoute)
	{
	case ELobbyRoute::ToggleReady:
		if (Entry.bIsHost)
			break;		

		Entry.bIsReady = !Entry.bIsReady;
		OnLobbyPlayersChanged.Broadcast();
		break;
		
	case ELobbyRoute::EnterGame:
		if (Entry.bIsHost == false)
			break;

		HandleEnterGameRequest();
		break;
		
	default:
		break;
	}
}

void ALobbyGameState::AddLobbyPlayer(const APlayerController* InPlayerController)
{
	if (HasAuthority() == false || InPlayerController == nullptr)
		return;
	
	FLobbyPlayerEntry& Entry = LobbyPlayers.AddDefaulted_GetRef();
	Entry.PlayerId = InPlayerController->PlayerState->GetPlayerId();
	Entry.PlayerName = InPlayerController->PlayerState->GetPlayerName();
	Entry.bIsHost = InPlayerController->IsLocalController();
	Entry.bIsReady = Entry.bIsHost;		
	
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

bool ALobbyGameState::IsAllReady() const
{
	for (const FLobbyPlayerEntry& Player : LobbyPlayers)
	{
		if (Player.bIsReady == false)
			return false;
	}
	return true;
}

void ALobbyGameState::RebuildIndexCache()
{
	PlayerIdToLobbyPlayerIndex.Reset();
	for (int32 i = 0; i < LobbyPlayers.Num(); i++)
	{
		PlayerIdToLobbyPlayerIndex.Add(LobbyPlayers[i].PlayerId, i);
	}
}

void ALobbyGameState::HandleEnterGameRequest()
{
	const bool bIsAllReady = IsAllReady();
	if (bIsAllReady == false)
		return;
	
	
}
