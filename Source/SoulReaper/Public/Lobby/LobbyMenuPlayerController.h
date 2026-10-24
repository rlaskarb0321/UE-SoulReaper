// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "Network/Request/RequestHandler.h"
#include "LobbyMenuPlayerController.generated.h"

class AGameStateBase;
class ALobbyGameState;
class ULobbyMenu;
class UShopSubsystem;
class UUISubsystem;
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

	// 지갑 조회(서버 GET)가 실패했을 때 띄울 팝업. 왼쪽 버튼 = 다시 시도, 오른쪽 버튼 = 아니오(행의 다음 팝업)
	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Shop", meta = (RowType = "/Script/SoulReaper.PopupTypeRow"))
	FDataTableRowHandle WalletLoadFailedPopupRow;

	// 재시도 한도(ShopSettings 의 MaxRetryCount)를 넘었을 때 띄울 안내 팝업. 확인 전용이며 동작은 행(DT)에 지정한다
	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Shop", meta = (RowType = "/Script/SoulReaper.PopupTypeRow"))
	FDataTableRowHandle WalletLoadGiveUpPopupRow;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<ULobbyMenu> LobbyMenuWidget;

	UPROPERTY()
	TObjectPtr<UShopSubsystem> ShopSubsystem;

	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

	// 지갑 조회가 연속으로 실패한 횟수 (성공하면 0으로 되돌린다)
	int32 CurrentRetryCount = 0;

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

	void RequestWalletLoad();
	void HandleWalletLoaded(const FShopResponse& Response);
};
