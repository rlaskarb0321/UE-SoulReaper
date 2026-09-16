// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MainMenuWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (PlayButton != nullptr)
	{
		PlayButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::HandlePlayClicked);
	}
	
	if (QuitButton != nullptr)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::HandleQuitClicked);
	}
}

void UMainMenuWidget::HandlePlayClicked()
{
	if (LobbyLevel.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[UMainMenuWidget::HandlePlayClicked] Lobby is not set"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidget::HandlePlayClicked] Open Level : %s"),
		*LobbyLevel.GetAssetName());
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevel);
}

void UMainMenuWidget::HandleQuitClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Quit Button Clicked"));
}
