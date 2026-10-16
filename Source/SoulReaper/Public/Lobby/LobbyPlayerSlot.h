// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerSlot.generated.h"

class UImage;
struct FLobbyPlayerEntry;
class UTextBlock;
/**
 * 
 */

UENUM(BlueprintType)
enum class ELobbySlotState : uint8
{
	Empty			UMETA(DisplayName = "Empty"),
	Client			UMETA(DisplayName = "Client"),
	Host			UMETA(DisplayName = "Host"),
	MAX				UMETA(Hidden)
};

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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlotBackground;

	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Lobby")
	TMap<ELobbySlotState, FSlateBrush> StateBrushes;
	
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
	static const FText& GetHostText()
	{
		static const FText Text = NSLOCTEXT("Lobby", "Host", "-Host-");
		return Text;
	}
	
private:
	void ApplySlotBackground(ELobbySlotState NewState);
};
