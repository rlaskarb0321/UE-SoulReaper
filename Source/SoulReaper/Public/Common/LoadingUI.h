// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingUI.generated.h"

/**
 * 서버 응답을 기다리는 동안 화면을 덮는 로딩 UI (WBP_LoadingUI 의 부모).
 * 화면에 올라가면 스피너 애니메이션을 무한 반복하고, 내려가면 멈춘다.
 */
UCLASS()
class SOULREAPER_API ULoadingUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> LoadingAnim;
};
