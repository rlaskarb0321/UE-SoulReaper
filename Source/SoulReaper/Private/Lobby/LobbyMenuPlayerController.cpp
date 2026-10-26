// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuPlayerController.h"

#include "Common/UISubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Lobby/ELobbyRoute.h"
#include "Lobby/LobbyGameState.h"
#include "Lobby/LobbyMenu.h"
#include "Shop/ShopSettings.h"
#include "Shop/ShopSubsystem.h"
#include "Shop/ShopTypes.h"


void ALobbyMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>();
	UISubsystem = GetGameInstance()->GetSubsystem<UUISubsystem>();

	// 로비에 들어오면 지갑을 불러온다. 결과 값은 상점 위젯이 서브시스템에서 받고, 여기서는 성공/실패 흐름만 다룬다
	RequestWalletLoad();

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

	// 클라에선 GameState 가 아직 오지 않았을 수 있으니, 없으면 GameStateSet 이벤트로 기다린다
	UWorld* World = GetWorld();
	if (ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>())
	{
		BindLobbyGameState(LobbyGS);
	}
	else
	{
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ALobbyMenuPlayerController::OnGameStateSet);
	}
}

void ALobbyMenuPlayerController::OnGameStateSet(AGameStateBase* NewGameState)
{
	ALobbyGameState* LobbyGS = Cast<ALobbyGameState>(NewGameState);
	if (LobbyGS == nullptr)
		return;

	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();

	BindLobbyGameState(LobbyGS);
}

void ALobbyMenuPlayerController::BindLobbyGameState(ALobbyGameState* LobbyGameState)
{
	BoundLobbyGameState = LobbyGameState;
	LobbyGameState->OnLobbyPlayersChanged.AddUniqueDynamic(this, &ALobbyMenuPlayerController::RefreshLobbyMenu);
	LobbyGameState->OnLobbyPhaseChanged.AddUniqueDynamic(this, &ALobbyMenuPlayerController::HandleLobbyPhaseChanged);

	// 바인딩 전에 이미 도착한 목록을 놓치지 않도록 한 번 직접 갱신
	RefreshLobbyMenu();
}

void ALobbyMenuPlayerController::HandleLobbyPhaseChanged()
{
	if (BoundLobbyGameState.IsValid() == false)
		return;

	const bool bCommitting = BoundLobbyGameState->GetPhase() == ELobbyPhase::Committing;

	// 결제 중에는 로비 화면 전체를 잠근다 (결제가 끝나거나 중단되면 다시 연다)
	if (LobbyMenuWidget != nullptr)
		LobbyMenuWidget->SetIsEnabled(bCommitting == false);

	if (bCommitting == false)
		return;

	// 내 컴퓨터의 상점 서브시스템으로 내 구매 내역을 결제하고, 결과를 서버에 보고한다
	if (ShopSubsystem == nullptr)
	{
		HandleRequest(static_cast<int32>(ELobbyRoute::CheckoutFailed));
		return;
	}

	FOnShopResponse OnDone;
	OnDone.BindUObject(this, &ALobbyMenuPlayerController::HandleCheckoutResponse);
	ShopSubsystem->Checkout(OnDone);
}

void ALobbyMenuPlayerController::HandleCheckoutResponse(const FShopResponse& Response)
{
	const bool bSucceeded = Response.NetworkResult == ENetworkResponse::Success;

	// 서버에는 성공/실패만 보고한다. 실패 이유는 이 컴퓨터의 플레이어에게만 알린다.
	HandleRequest(static_cast<int32>(bSucceeded ? ELobbyRoute::CheckoutDone : ELobbyRoute::CheckoutFailed));
	if (bSucceeded)
		return;

	const FString ErrorCode = ShopSubsystem != nullptr ? ShopSubsystem->GetErrorCode(Response.NetworkResult) : FString();
	UE_LOG(LogTemp, Warning, TEXT("[ALobbyMenuPlayerController::HandleCheckoutResponse] Checkout failed (%s)"), *ErrorCode);

	if (UISubsystem == nullptr)
		return;

	FFormatNamedArguments Args;
	Args.Add(TEXT("ErrorCode"), FText::FromString(ErrorCode));
	UISubsystem->ShowPopup(CheckoutFailedPopupRow.RowName, FOnPopupButtonClicked(), FOnPopupButtonClicked(), Args);
}

void ALobbyMenuPlayerController::RequestWalletLoad()
{
	if (ShopSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuPlayerController::RequestWalletLoad] ShopSubsystem is nullptr"));
		return;
	}

	// 응답이 올 때까지 로딩 UI 로 화면을 덮는다
	if (UISubsystem != nullptr)
	{
		UISubsystem->ShowLoading();
	}

	FOnShopResponse OnDone;
	OnDone.BindUObject(this, &ALobbyMenuPlayerController::HandleWalletLoaded);
	ShopSubsystem->LoadWallet(OnDone);
}

void ALobbyMenuPlayerController::HandleWalletLoaded(const FShopResponse& Response)
{
	if (UISubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuPlayerController::HandleWalletLoaded] UISubsystem is nullptr"));
		return;
	}

	// 성공이든 실패든 응답이 왔으니 로딩 UI 는 먼저 내린다 (실패 팝업이 로딩 뒤에 가려지지 않게)
	UISubsystem->HideLoading();

	// 성공하면 지갑 값은 서브시스템의 방송으로 이미 화면에 반영됐다
	if (Response.NetworkResult == ENetworkResponse::Success)
	{
		CurrentRetryCount = 0;
		return;
	}

	const FString ErrorCode = ShopSubsystem != nullptr ? ShopSubsystem->GetErrorCode(Response.NetworkResult) : FString();
	UE_LOG(LogTemp, Warning, TEXT("[ALobbyMenuPlayerController::HandleWalletLoaded] Wallet load failed (%s)"), *ErrorCode);

	// 한도를 넘으면 더 시도할 수 없다. 재시도 델리게이트를 넘기지 않아야 행에 지정된 동작(메인 메뉴 이동 등)이 실행된다.
	CurrentRetryCount++;
	if (CurrentRetryCount >= GetDefault<UShopSettings>()->MaxRetryCount)
	{
		UISubsystem->ShowPopup(WalletLoadGiveUpPopupRow.RowName);
		return;
	}

	// 왼쪽(다시 시도): 지갑 조회를 다시 요청한다.
	// 오른쪽(아니오): 넘기지 않는다. 행에 지정된 다음 팝업이 뜬다.
	FOnPopupButtonClicked OnRetry;
	OnRetry.BindUObject(this, &ALobbyMenuPlayerController::RequestWalletLoad);

	FFormatNamedArguments Args;
	Args.Add(TEXT("ErrorCode"), FText::FromString(ErrorCode));
	UISubsystem->ShowPopup(WalletLoadFailedPopupRow.RowName, OnRetry, FOnPopupButtonClicked(), Args);
}

void ALobbyMenuPlayerController::RefreshLobbyMenu()
{
	if (LobbyMenuWidget == nullptr || BoundLobbyGameState.IsValid() == false)
		return;

	LobbyMenuWidget->RefreshPlayers(BoundLobbyGameState->GetLobbyPlayers());
}

void ALobbyMenuPlayerController::HandleRequest_Implementation(int32 InPayload)
{
	ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (LobbyGameState == nullptr)
		return;
	
	LobbyGameState->HandleLobbyRequest(PlayerState->GetPlayerId(), InPayload);
}

void ALobbyMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 응답을 기다리는 중에 로비가 끝나면 HandleWalletLoaded 가 불리지 않는다. 로딩 UI 가 남지 않게 여기서 내린다.
	if (UISubsystem != nullptr)
	{
		UISubsystem->HideLoading();
	}

	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();

	if (BoundLobbyGameState.IsValid())
	{
		BoundLobbyGameState->OnLobbyPlayersChanged.RemoveDynamic(this, &ALobbyMenuPlayerController::RefreshLobbyMenu);
		BoundLobbyGameState->OnLobbyPhaseChanged.RemoveDynamic(this, &ALobbyMenuPlayerController::HandleLobbyPhaseChanged);
	}

	Super::EndPlay(EndPlayReason);
}