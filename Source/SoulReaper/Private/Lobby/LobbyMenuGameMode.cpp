// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuGameMode.h"

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
