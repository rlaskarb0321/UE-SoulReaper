// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuPlayerController.h"

#include "Lobby/LobbyGameState.h"
#include "Lobby/LobbyMenu.h"
#include "Lobby/LobbyPlayerState.h"


void ALobbyMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	if (LobbyMenuWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyMenuPlayerController::BeginPlay] LobbyMenuWidgetClass is nullptr"));
		return;
	}

	LobbyMenuWidget = CreateWidget<ULobbyMenu>(this, LobbyMenuWidgetClass);
	if (LobbyMenuWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyMenuPlayerController::BeginPlay] LobbyMenuWidget is not created"));
		return;
	}

	LobbyMenuWidget->AddToViewport();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	// 클라에선 GameState 가 아직 오지 않았을 수 있으니, 없으면 GameStateSet 이벤트로 기다린다
	UWorld* World = GetWorld();
	if (ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>())
	{
		BindLobbyGameState(LobbyGS);
	}
	else
	{
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ALobbyMenuPlayerController::OnGameStateSet);
	}
}

void ALobbyMenuPlayerController::OnGameStateSet(AGameStateBase* NewGameState)
{
	ALobbyGameState* LobbyGS = Cast<ALobbyGameState>(NewGameState);
	if (LobbyGS == nullptr)
		return;

	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();

	BindLobbyGameState(LobbyGS);
}

void ALobbyMenuPlayerController::BindLobbyGameState(ALobbyGameState* LobbyGameState)
{
	BoundLobbyGameState = LobbyGameState;
	LobbyGameState->OnLobbyPlayersChanged.AddUniqueDynamic(this, &ALobbyMenuPlayerController::RefreshLobbyMenu);

	// 바인딩 전에 이미 도착한 목록을 놓치지 않도록 한 번 직접 갱신
	RefreshLobbyMenu();
}

void ALobbyMenuPlayerController::RefreshLobbyMenu()
{
	if (LobbyMenuWidget == nullptr || BoundLobbyGameState.IsValid() == false)
		return;

	LobbyMenuWidget->RefreshPlayers(BoundLobbyGameState->GetLobbyPlayers());
}

void ALobbyMenuPlayerController::Server_SetReady_Implementation(bool bInIsReady)
{
	ALobbyPlayerState* LobbyPlayerState = GetPlayerState<ALobbyPlayerState>();
	if (LobbyPlayerState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuPlayerController::Server_SetReady_Implementation] LobbyPlayerState is nullptr"));
		return;
	}

	LobbyPlayerState->SetIsReady(bInIsReady);
}

void ALobbyMenuPlayerController::RequestSetReady(bool bInIsReady)
{
	Server_SetReady(bInIsReady);
}

void ALobbyMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();

	if (BoundLobbyGameState.IsValid())
	{
		BoundLobbyGameState->OnLobbyPlayersChanged.RemoveDynamic(this, &ALobbyMenuPlayerController::RefreshLobbyMenu);
	}

	Super::EndPlay(EndPlayReason);
}
