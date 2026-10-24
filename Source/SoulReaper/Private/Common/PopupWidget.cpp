// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/PopupWidget.h"

#include "Common/PopupTypes.h"
#include "Common/UISubsystem.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UISubsystem = GameInstance->GetSubsystem<UUISubsystem>();
	}

	LeftButton->OnClicked.AddUniqueDynamic(this, &UPopupWidget::HandleLeftClicked);
	RightButton->OnClicked.AddUniqueDynamic(this, &UPopupWidget::HandleRightClicked);
}

void UPopupWidget::SetData(const FPopupTypeRow& InRow, FOnPopupButtonClicked InOnLeft, FOnPopupButtonClicked InOnRight)
{
	OnLeft = InOnLeft;
	OnRight = InOnRight;

	LeftNextPopupRow = InRow.LeftNextPopupRow;
	RightNextPopupRow = InRow.RightNextPopupRow;
	bCloseOnLeftClick = InRow.bCloseOnLeftClick;
	bCloseOnRightClick = InRow.bCloseOnRightClick;

	TitleText->SetText(InRow.Title);
	ContentText->SetText(InRow.Content);
	LeftButtonText->SetText(InRow.LeftText);
	RightButtonText->SetText(InRow.RightText);
	RightSizeBox->SetVisibility(InRow.bIsConfirmOnly ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

void UPopupWidget::HandleLeftClicked()
{
	HandleButtonClicked(OnLeft, bCloseOnLeftClick, LeftNextPopupRow);
}

void UPopupWidget::HandleRightClicked()
{
	HandleButtonClicked(OnRight, bCloseOnRightClick, RightNextPopupRow);
}

void UPopupWidget::HandleButtonClicked(FOnPopupButtonClicked Callback, bool bClose, FName NextPopupRow)
{
	if (bClose)
	{
		OnLeft.Unbind();
		OnRight.Unbind();
		RemoveFromParent();
	}

	Callback.ExecuteIfBound();

	if (NextPopupRow.IsNone())
		return;

	if (UISubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UPopupWidget::HandleButtonClicked] UISubsystem is nullptr"));
		return;
	}

	UISubsystem->ShowPopup(NextPopupRow);
}
