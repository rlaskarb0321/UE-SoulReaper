// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Network/Request/RequestHandler.h"
#include "LobbyMenuPlayerController.generated.h"

class AGameStateBase;
class ALobbyGameState;
class ULobbyMenu;
class UShopSubsystem;
struct FShopResponse;
/**
 *
 */
UCLASS()
class SOULREAPER_API ALobbyMenuPlayerController : public APlayerController, public IRequestHandler
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	virtual void HandleRequest(int32 InPayload) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Room Enter")
	TSubclassOf<ULobbyMenu> LobbyMenuWidgetClass;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<ULobbyMenu> LobbyMenuWidget;

	UPROPERTY()
	TObjectPtr<UShopSubsystem> ShopSubsystem;

	TWeakObjectPtr<ALobbyGameState> BoundLobbyGameState;
	FDelegateHandle GameStateSetHandle;

private:
	void OnGameStateSet(AGameStateBase* NewGameState);
	void BindLobbyGameState(ALobbyGameState* LobbyGameState);

	UFUNCTION()
	void RefreshLobbyMenu();

	UFUNCTION()
	void HandleLobbyPhaseChanged();

	void HandleCheckoutResponse(const FShopResponse& Response);
};
