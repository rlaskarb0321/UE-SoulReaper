// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/Shop/LobbyShop.h"

#include "Components/TextBlock.h"
#include "Shop/ShopSubsystem.h"

void ULobbyShop::InitializeInputComponent()
{
	Super::InitializeInputComponent();
	
	UShopSubsystem* ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>();
	if (ShopSubsystem == nullptr)
		return;
	
	HandleGoldChanged(ShopSubsystem->GetGold());
	ShopSubsystem->OnGoldChanged.AddUObject(this, &ULobbyShop::HandleGoldChanged);
}

void ULobbyShop::HandleGoldChanged(int32 NewGold)
{
	GoldAmount->SetText(FText::AsNumber(NewGold));
}
