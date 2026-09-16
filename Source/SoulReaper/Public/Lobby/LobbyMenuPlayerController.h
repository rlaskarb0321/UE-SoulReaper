// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyMenuPlayerController.generated.h"

class ULobbyMenu;
/**
 * 
 */
UCLASS()
class SOULREAPER_API ALobbyMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Room Enter")
	TSubclassOf<ULobbyMenu> LobbyMenuWidgetClass;
	
private:

	UPROPERTY()
	TObjectPtr<ULobbyMenu> LobbyMenuWidget;
};
