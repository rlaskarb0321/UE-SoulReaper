// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenu.generated.h"

class UWidgetSwitcher;
class UButton;
class ULobbyPlayerSlot;
class UVerticalBox;
struct FLobbyPlayerEntry;
/**
 *
 */

UENUM(BlueprintType)
enum class EPlayButtonSwitcher : uint8
{
	Client	= 0			UMETA(DisplayName = "Client"),
	Host	= 1			UMETA(DisplayName = "Host"),
	MAX					UMETA(Hidden)
};

UCLASS()
class SOULREAPER_API ULobbyMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshPlayers(const TArray<FLobbyPlayerEntry>& Entries);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Lobby")
	TSoftObjectPtr<UWorld> MainMenuLevel;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> PlayButtonSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeaveButton;
	
	UPROPERTY()
	TArray<TObjectPtr<ULobbyPlayerSlot>> PlayerSlots;
	
	virtual void NativeOnInitialized() override;
	
private:
	UFUNCTION()
	void HandleReadyClicked();
	
	UFUNCTION()
	void HandleHostButtonClicked();
	
	UFUNCTION()
	void HandleLeaveButtonClicked();
};
