// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class SOULREAPER_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "SoulReaper|Menu")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;
	
protected:
	
	UFUNCTION()
	void HandlePlayClicked();
	
	UFUNCTION()
	void HandleQuitClicked();
};
