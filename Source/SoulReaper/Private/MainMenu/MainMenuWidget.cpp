// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu/MainMenuWidget.h"

#include "Components/Button.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Play Button Clicked"));
}

void UMainMenuWidget::HandleQuitClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Quit Button Clicked"));
}
