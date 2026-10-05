// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameState.h"

#include "Lobby/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbyPlayers);
}

void ALobbyGameState::RebuildLobbyPlayers()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ALobbyGameState::RebuildLobbyPlayers] 클라이언트에서 호출됨 — 무시한다"));
		return;
	}

	LobbyPlayers.Reset();
	for (APlayerState* PlayerState : PlayerArray)
	{
		const ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState);
		if (LobbyPlayerState == nullptr)
			continue;

		FLobbyPlayerEntry& Entry = LobbyPlayers.AddDefaulted_GetRef();
		Entry.PlayerName = LobbyPlayerState->GetPlayerName();
		Entry.bIsReady = LobbyPlayerState->IsReady();
	}

	// 서버(호스트)는 RepNotify 가 자동으로 불리지 않으므로 직접 호출
	OnRep_LobbyPlayers();
}

void ALobbyGameState::OnRep_LobbyPlayers()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] 로비 변경 신호 (%d명)"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"), LobbyPlayers.Num());
	OnLobbyPlayersChanged.Broadcast();
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	// 클라에서도 PS 파괴 시 불리지만, 목록은 서버가 복제해주므로 서버에서만 다시 만든다
	if (HasAuthority())
	{
		RebuildLobbyPlayers();
	}
}
