// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerSlot.generated.h"

struct FLobbyPlayerEntry;
class UTextBlock;
/**
 * 
 */
UCLASS()
class SOULREAPER_API ULobbyPlayerSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntry(const FLobbyPlayerEntry& Entry);
	void SetEmpty();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyIndicator;
	
	static const FText& GetReadyText()
	{
		static const FText Text = NSLOCTEXT("Lobby", "Ready", "-Ready-");
		return Text;
	}
	static const FText& GetUnreadyText()
	{
		static const FText Text = NSLOCTEXT("Lobby", "Unready", "-Unready-");
		return Text;
	}
};
