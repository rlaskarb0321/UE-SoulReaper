// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PopupSettings.generated.h"

class ULoadingUI;
class UPopupWidget;
/**
 *
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Popup Widget"))
class SOULREAPER_API UPopupWidgetSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Popup Widget")
	TSoftObjectPtr<UDataTable> PopupTable;

	// 팝업으로 띄울 위젯 (WBP_PopUp)
	UPROPERTY(Config, EditAnywhere, Category = "Popup Widget")
	TSoftClassPtr<UPopupWidget> PopupWidgetClass;

	// 서버 응답을 기다릴 때 띄울 로딩 UI (WBP_LoadingUI)
	UPROPERTY(Config, EditAnywhere, Category = "Popup Widget")
	TSoftClassPtr<ULoadingUI> LoadingWidgetClass;
};
