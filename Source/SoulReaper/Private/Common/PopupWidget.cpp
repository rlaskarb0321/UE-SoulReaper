// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/PopupWidget.h"

#include "Common/PopupTypes.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	LeftButton->OnClicked.AddUniqueDynamic(this, &UPopupWidget::HandleLeftClicked);
	RightButton->OnClicked.AddUniqueDynamic(this, &UPopupWidget::HandleRightClicked);
}

void UPopupWidget::SetData(const FPopupTypeRow& InRow, FOnPopupButtonClicked InOnLeft, FOnPopupButtonClicked InOnRight)
{
	OnLeft = InOnLeft;
	OnRight = InOnRight;

	// 팝업 위젯은 재사용되지 않지만, 값이 남지 않도록 매번 덮어쓴다
	LeftNextPopupRow = InRow.LeftNextPopupRow;
	RightNextPopupRow = InRow.RightNextPopupRow;

	TitleText->SetText(InRow.Title);
	ContentText->SetText(InRow.Content);
	LeftButtonText->SetText(InRow.LeftText);
	RightButtonText->SetText(InRow.RightText);
	RightSizeBox->SetVisibility(InRow.bIsConfirmOnly ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

void UPopupWidget::HandleLeftClicked()
{
	const FOnPopupButtonClicked Callback = OnLeft;
	OnLeft.Unbind();
	OnRight.Unbind();

	RemoveFromParent();
	Callback.ExecuteIfBound();
}

void UPopupWidget::HandleRightClicked()
{
	const FOnPopupButtonClicked Callback = OnRight;
	OnLeft.Unbind();
	OnRight.Unbind();

	RemoveFromParent();
	Callback.ExecuteIfBound();
}

