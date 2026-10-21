// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Lobby/LobbyTypes.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayersChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPhaseChanged);

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

	UPROPERTY(BlueprintAssignable, Category = "SoulReaper|Lobby")
	FOnLobbyPhaseChanged OnLobbyPhaseChanged;

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void HandleLobbyRequest(int32 InRequesterId, int32 InPayload);
	void AddLobbyPlayer(const APlayerController* InPlayerController);
	
	const TArray<FLobbyPlayerEntry>& GetLobbyPlayers() const { return LobbyPlayers; }
	ELobbyPhase GetPhase() const { return Phase; }

private:
	// 결제 응답을 기다리는 최대 시간. 넘기면 응답 못 한 플레이어를 실패로 보고 중단한다.
	static constexpr float CommitTimeoutSeconds = 10.f;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayers)
	TArray<FLobbyPlayerEntry> LobbyPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_Phase)
	ELobbyPhase Phase = ELobbyPhase::Waiting;

	TMap<int32, int32> PlayerIdToLobbyPlayerIndex;
	FTimerHandle CommitTimeoutHandle;

private:
	UFUNCTION()
	void OnRep_LobbyPlayers();

	UFUNCTION()
	void OnRep_Phase();

	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	void RebuildIndexCache();
	void HandleEnterGameRequest();
	void EvaluateCommit();
	void HandleCommitTimeout();
	bool IsAllReady() const;
};
