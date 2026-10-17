// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyShop.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class SOULREAPER_API ULobbyShop : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldAmount;
	
	virtual void InitializeInputComponent() override;
	
private:
	void HandleGoldChanged(int32 NewGold);
};
