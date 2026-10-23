// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/LoadingUI.h"

#include "Animation/WidgetAnimation.h"

void ULoadingUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoadingAnim == nullptr)
		return;

	// NumLoopsToPlay 가 0 이면 무한 반복
	PlayAnimation(LoadingAnim, 0.f, 0);
}

void ULoadingUI::NativeDestruct()
{
	if (LoadingAnim != nullptr)
	{
		StopAnimation(LoadingAnim);
	}

	Super::NativeDestruct();
}
