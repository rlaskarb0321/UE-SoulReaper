// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyMenuPlayerController.generated.h"

class AGameStateBase;
class ALobbyGameState;
class ULobbyMenu;
/**
 *
 */
UCLASS()
class SOULREAPER_API ALobbyMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Room Enter")
	TSubclassOf<ULobbyMenu> LobbyMenuWidgetClass;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<ULobbyMenu> LobbyMenuWidget;

	TWeakObjectPtr<ALobbyGameState> BoundLobbyGameState;
	FDelegateHandle GameStateSetHandle;

	void OnGameStateSet(AGameStateBase* NewGameState);
	void BindLobbyGameState(ALobbyGameState* LobbyGameState);

	UFUNCTION()
	void RefreshLobbyMenu();
};
