// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuGameMode.h"

#include "Engine/World.h"
#include "Lobby/LobbyGameState.h"
#include "Lobby/LobbyMenuPlayerController.h"
#include "Lobby/LobbyPlayerState.h"

ALobbyMenuGameMode::ALobbyMenuGameMode()
{
	PlayerControllerClass = ALobbyMenuPlayerController::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
	DefaultPawnClass = nullptr;
}

void ALobbyMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
	{
		LobbyGS->AddLobbyPlayer(NewPlayer);
	}
}

void ALobbyMenuGameMode::StartGame()
{
	if (GameLevel.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuGameMode::StartGame] GameLevel is not set"));
		return;
	}

	// 하드 트래블은 URL 에 listen 옵션이 있어야 호스트가 계속 리슨 서버로 남는다
	const FString Url = GameLevel.ToSoftObjectPath().GetLongPackageName() + TEXT("?listen");
	GetWorld()->ServerTravel(Url);
}
