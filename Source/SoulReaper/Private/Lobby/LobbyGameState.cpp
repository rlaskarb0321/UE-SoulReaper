// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameState.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Lobby/ELobbyRoute.h"
#include "Lobby/LobbyMenuGameMode.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbyPlayers);
	DOREPLIFETIME(ALobbyGameState, Phase);
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

	case ELobbyRoute::CheckoutDone:
	case ELobbyRoute::CheckoutFailed:
		// 결제 단계가 아니거나, 이미 보고한 플레이어의 보고는 무시한다
		if (Phase != ELobbyPhase::Committing || Entry.CheckoutState != ECheckoutState::Pending)
			break;

		Entry.CheckoutState = (LobbyRoute == ELobbyRoute::CheckoutDone) ? ECheckoutState::Done : ECheckoutState::Failed;
		OnLobbyPlayersChanged.Broadcast();
		EvaluateCommit();
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

void ALobbyGameState::OnRep_Phase()
{
	OnLobbyPhaseChanged.Broadcast();
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
	// 전원 Ready 가 아니거나, 이미 결제 단계라면 무시 (EnterGame 연타 방지)
	if (IsAllReady() == false || Phase != ELobbyPhase::Waiting)
		return;

	// 결제 단계 시작: 모든 플레이어를 결제 대기 상태로 만든다
	Phase = ELobbyPhase::Committing;
	for (FLobbyPlayerEntry& Entry : LobbyPlayers)
	{
		Entry.CheckoutState = ECheckoutState::Pending;
	}

	// 응답이 오지 않는 플레이어가 있어도 무한정 기다리지 않도록 타임아웃을 건다
	GetWorldTimerManager().SetTimer(CommitTimeoutHandle, this,
		&ALobbyGameState::HandleCommitTimeout, CommitTimeoutSeconds, false);

	// 서버(호스트)는 값을 직접 바꾼 쪽이라 OnRep 이 자동으로 불리지 않으므로 직접 호출한다
	OnRep_Phase();
	OnRep_LobbyPlayers();
}

void ALobbyGameState::EvaluateCommit()
{
	if (Phase != ELobbyPhase::Committing)
		return;

	bool bAnyFailed = false;
	bool bAllDone = true;
	for (const FLobbyPlayerEntry& Entry : LobbyPlayers)
	{
		if (Entry.CheckoutState == ECheckoutState::Failed)
			bAnyFailed = true;

		if (Entry.CheckoutState != ECheckoutState::Done)
			bAllDone = false;
	}

	// 한 명이라도 실패 → 이동하지 않고 결제 단계를 중단한다
	if (bAnyFailed)
	{
		GetWorldTimerManager().ClearTimer(CommitTimeoutHandle);

		Phase = ELobbyPhase::Waiting;
		for (FLobbyPlayerEntry& Entry : LobbyPlayers)
		{
			Entry.CheckoutState = ECheckoutState::None;
		}

		OnRep_Phase();
		OnRep_LobbyPlayers();
		return;
	}

	// 전원 완료 → 게임 시작
	if (bAllDone)
	{
		GetWorldTimerManager().ClearTimer(CommitTimeoutHandle);

		if (ALobbyMenuGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyMenuGameMode>())
			LobbyGameMode->StartGame();
	}
}

void ALobbyGameState::HandleCommitTimeout()
{
	// 아직 응답하지 않은 플레이어는 실패로 처리하고, 결과를 다시 판단한다
	for (FLobbyPlayerEntry& Entry : LobbyPlayers)
	{
		if (Entry.CheckoutState == ECheckoutState::Pending)
			Entry.CheckoutState = ECheckoutState::Failed;
	}

	EvaluateCommit();
}
