// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenu.generated.h"

// UENUM(BlueprintType)
// enum class ELobbyRoute : uint8
// {
// 	None					UMETA(DisplayName = "None"),
// 	ToggleReady             UMETA(DisplayName = "ToggleReady"),
// 	
// 	MAX						UMETA(Hidden)
// };

class UButton;
class ULobbyPlayerSlot;
class UVerticalBox;
struct FLobbyPlayerEntry;
/**
 *
 */
UCLASS()
class SOULREAPER_API ULobbyMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshPlayers(const TArray<FLobbyPlayerEntry>& Entries);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListBox;
	
	UPROPERTY()
	TArray<TObjectPtr<ULobbyPlayerSlot>> PlayerSlots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;
	
	virtual void NativeOnInitialized() override;
	
private:
	UFUNCTION()
	void HandleReadyClicked();
};
