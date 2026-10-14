// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenu.generated.h"

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
	
	virtual void NativeOnInitialized() override;
};
