// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuGameMode.h"

#include "Lobby/LobbyMenuPlayerController.h"

ALobbyMenuGameMode::ALobbyMenuGameMode()
{
	PlayerControllerClass = ALobbyMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
