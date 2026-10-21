// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/PopupWidget.h"

#include "Common/PopupTypes.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UPopupWidget::SetData(const FPopupTypeRow& InRow)
{
	TitleText->SetText(InRow.Title);
	ContentText->SetText(InRow.Content);
	LeftButtonText->SetText(InRow.LeftText);
	RightButtonText->SetText(InRow.RightText);

	// 확인만 있는 팝업이면 오른쪽 버튼을 자리째 없앤다. 보일 때는 SizeBox 의 기본 가시성으로 되돌린다.
	RightSizeBox->SetVisibility(InRow.bIsConfirmOnly ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

