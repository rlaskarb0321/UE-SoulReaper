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

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void AddLobbyPlayer(const APlayerState* PlayerState);
	
	const TArray<FLobbyPlayerEntry>& GetLobbyPlayers() const { return LobbyPlayers; }

private:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayers)
	TArray<FLobbyPlayerEntry> LobbyPlayers;

	TMap<int32, int32> PlayerIdToLobbyPlayerIndex;
	
private:
	UFUNCTION()
	void OnRep_LobbyPlayers();

	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	void RebuildIndexCache();
};
