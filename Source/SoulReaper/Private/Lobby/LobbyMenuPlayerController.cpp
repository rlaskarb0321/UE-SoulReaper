// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyMenuPlayerController.h"

#include "Lobby/LobbyGameState.h"
#include "Lobby/LobbyMenu.h"
#include "Lobby/LobbyPlayerState.h"
#include "Lobby/LobbyTypes.h"


void ALobbyMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
		return;
	
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
	BindToLobbyGameState();
}

void ALobbyMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.RemoveAll(this);

		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			LobbyGameState->OnLobbyPlayersChanged.RemoveDynamic(
				this, &ALobbyMenuPlayerController::HandleLobbyPlayersChanged);
		}
	}

	
	Super::EndPlay(EndPlayReason);
}

void ALobbyMenuPlayerController::BindToLobbyGameState()
{
	if (AGameStateBase* ExistingGameState = GetWorld()->GetGameState())
	{
		HandleGameStateSet(ExistingGameState);
		return;
	}
	
	GetWorld()->GameStateSetEvent.AddUObject(this, &ALobbyMenuPlayerController::HandleGameStateSet);
}

void ALobbyMenuPlayerController::HandleGameStateSet(AGameStateBase* NewGameState)
{
	if (ALobbyGameState* LobbyGameState = Cast<ALobbyGameState>(NewGameState))
	{
		LobbyGameState->OnLobbyPlayersChanged.AddUniqueDynamic(this, &ALobbyMenuPlayerController::HandleLobbyPlayersChanged);
		return;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuPlayerController::HandleGameStateSet] LobbyGameState is nullptr"));
}

void ALobbyMenuPlayerController::HandleLobbyPlayersChanged()
{
	if (LobbyMenuWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyMenuPlayerController::HandleLobbyPlayersChanged] LobbyMenuWidget is not created"));
		return;
	}
	
	const ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (LobbyGameState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuPlayerController::HandleLobbyPlayersChanged] LobbyGameState is nullptr"))
		return;
	}

	TArray<FLobbyPlayerEntry> Entries;
	Entries.Reserve(LobbyGameState->PlayerArray.Num());
	
	for (const APlayerState* CurrentPlayerState : LobbyGameState->PlayerArray)
	{
		const ALobbyPlayerState* LobbyPlayerState = Cast<const ALobbyPlayerState>(CurrentPlayerState);
		if (LobbyPlayerState == nullptr)
			continue;

		FLobbyPlayerEntry& Entry = Entries.AddDefaulted_GetRef();
		
		Entry.PlayerName = LobbyPlayerState->GetPlayerName();
		Entry.bIsReady = LobbyPlayerState->IsReady();
	}
	
	LobbyMenuWidget->RefreshPlayers(Entries);
}

void ALobbyMenuPlayerController::Server_SetReady_Implementation(bool bInIsReady)
{
	ALobbyPlayerState* LobbyPlayerState = GetPlayerState<ALobbyPlayerState>();
	if (LobbyPlayerState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyMenuPlayerController::Server_SetReady_Implementation] LobbyPlayerState is nullptr"));
		return;
	}
	
	LobbyPlayerState->SetIsReady(bInIsReady);
}

void ALobbyMenuPlayerController::RequestSetReady(bool bInIsReady)
{
	Server_SetReady(bInIsReady);
}
