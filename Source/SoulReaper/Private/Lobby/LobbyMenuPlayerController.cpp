// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuPlayerController.h"

#include "Lobby/LobbyMenu.h"

void ALobbyMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
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
