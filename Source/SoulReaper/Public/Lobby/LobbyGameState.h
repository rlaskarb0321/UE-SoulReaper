// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Lobby/LobbyTypes.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayersChanged);

/**
 *
 */
UCLASS()
class SOULREAPER_API ALobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "SoulReaper|Lobby")
	FOnLobbyPlayersChanged OnLobbyPlayersChanged;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FLobbyPlayerEntry>& GetLobbyPlayers() const { return LobbyPlayers; }

	// 서버 전용. PlayerArray 를 읽어 LobbyPlayers 를 다시 만든다.
	void RebuildLobbyPlayers();

private:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayers)
	TArray<FLobbyPlayerEntry> LobbyPlayers;

	UFUNCTION()
	void OnRep_LobbyPlayers();

	virtual void RemovePlayerState(APlayerState* PlayerState) override;
};
