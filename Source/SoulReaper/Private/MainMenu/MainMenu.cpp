// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MainMenu.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton != nullptr)
	{
		PlayButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::HandlePlayClicked);
	}

	if (QuitButton != nullptr)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::HandleQuitClicked);
	}
}

void UMainMenu::HandlePlayClicked()
{
	if (LobbyLevel.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[UMainMenu::HandlePlayClicked] Lobby is not set"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UMainMenu::HandlePlayClicked] Open Level : %s"),
		*LobbyLevel.GetAssetName());
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevel);
}

void UMainMenu::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame
	(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
