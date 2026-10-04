// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuPlayerController.h"

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
