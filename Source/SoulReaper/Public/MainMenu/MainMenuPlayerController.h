// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenu;
/**
 *
 */
UCLASS()
class SOULREAPER_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Menu")
	TSubclassOf<UMainMenu> MainMenuWidgetClass;

private:

	UPROPERTY()
	TObjectPtr<UMainMenu> MainMenuWidget;
};
