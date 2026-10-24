// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuPlayerController.h"

#include "GameFramework/PlayerState.h"
#include "Lobby/ELobbyRoute.h"
#include "Lobby/LobbyGameState.h"
#include "Lobby/LobbyMenu.h"
#include "Shop/ShopSubsystem.h"
#include "Shop/ShopTypes.h"


void ALobbyMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>();

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
	const bool bSucceeded = Response.Result == EShopResult::Success;
	HandleRequest(static_cast<int32>(bSucceeded ? ELobbyRoute::CheckoutDone : ELobbyRoute::CheckoutFailed));
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