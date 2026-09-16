// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MenuGameMode.h"

#include "MainMenu/MenuPlayerController.h"

AMenuGameMode::AMenuGameMode()
{
	PlayerControllerClass = AMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
