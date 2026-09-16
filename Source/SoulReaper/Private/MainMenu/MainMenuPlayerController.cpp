// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MainMenuPlayerController.h"

#include "MainMenu/MainMenu.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	if (MainMenuWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[AMainMenuPlayerController::BeginPlay()] MainMenuWidgetClass is nullptr !!"));
		return;
	}

	MainMenuWidget = CreateWidget<UMainMenu>(this, MainMenuWidgetClass);
	if (MainMenuWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[AMainMenuPlayerController::BeginPlay()] MainMenuWidget is not created"));
		return;
	}

	MainMenuWidget->AddToViewport();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}
