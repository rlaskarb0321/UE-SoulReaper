// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MenuPlayerController.h"

#include "MainMenu/MainMenuWidget.h"

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
		return;
	
	if (MainMenuWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[AMenuPlayerController::BeginPlay()] MainMenuWidgetClass is nullptr !!"));
		return;
	}
	
	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
	if (MainMenuWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[AMenuPlayerController::BeginPlay()] MainMenuWidget is not created"));
		return;
	}
	
	MainMenuWidget->AddToViewport();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}
