// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerState.h"

#include "Lobby/LobbyGameState.h"
#include "Net/UnrealNetwork.h"

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
}

void ALobbyPlayerState::SetIsReady(bool bInIsReady)
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ALobbyPlayerState::SetIsReady] 클라이언트에서 호출됨 — 무시한다"));
		return;
	}
	
	bIsReady = bInIsReady;
	OnRep_IsReady();
}

void ALobbyPlayerState::OnRep_IsReady()
{
	if (ALobbyGameState* LobbyGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGS->NotifyLobbyPlayersChanged();
	}
}
