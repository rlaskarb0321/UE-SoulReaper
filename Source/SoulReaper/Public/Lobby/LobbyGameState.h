// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Lobby/LobbyTypes.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayersChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPhaseChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCheckoutFailed, int32, FailedPlayerId);

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

	// 결제가 실패해서 중단되었을 때 모든 플레이어에게 알린다. 실패한 플레이어를 알 수 없으면 FailedPlayerId 는 INDEX_NONE
	UPROPERTY(BlueprintAssignable, Category = "SoulReaper|Lobby")
	FOnLobbyCheckoutFailed OnLobbyCheckoutFailed;

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void HandleLobbyRequest(int32 InRequesterId, int32 InPayload);

	void AddLobbyPlayer(const APlayerController* InPlayerController);
	
	const TArray<FLobbyPlayerEntry>& GetLobbyPlayers() const { return LobbyPlayers; }
	ELobbyPhase GetPhase() const { return Phase; }

private:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayers)
	TArray<FLobbyPlayerEntry> LobbyPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_Phase)
	ELobbyPhase Phase = ELobbyPhase::Waiting;

	TMap<int32, int32> PlayerIdToLobbyPlayerIndex;
	FTimerHandle CommitTimeoutHandle;

	// 이번 결제 단계에서 처음 실패를 보고한 플레이어. 기록용이 아니라 중단을 알릴 때 쓰고 바로 되돌린다.
	int32 FirstFailedPlayerId = INDEX_NONE;

private:
	UFUNCTION()
	void OnRep_LobbyPlayers();

	UFUNCTION()
	void OnRep_Phase();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CheckoutFailed(int32 InFailedPlayerId);

	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	void RebuildIndexCache();
	void HandleEnterGameRequest();
	void EvaluateCommit();
	void HandleCommitTimeout();
	bool IsAllReady() const;
};
