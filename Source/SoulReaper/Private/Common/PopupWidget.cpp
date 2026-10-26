// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/PopupWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Common/PopupTypes.h"
#include "Common/UISubsystem.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"

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

void UPopupWidget::SetData(const FPopupTypeRow& InRow, FOnPopupButtonClicked InOnLeft, FOnPopupButtonClicked InOnRight,
	const FFormatNamedArguments& InArgs)
{
	OnLeft = InOnLeft;
	OnRight = InOnRight;

	LeftNextPopupRow = InRow.LeftNextPopupRow;
	RightNextPopupRow = InRow.RightNextPopupRow;
	bCloseOnLeftClick = InRow.bCloseOnLeftClick;
	bCloseOnRightClick = InRow.bCloseOnRightClick;

	// 채울 인자가 없으면 문구를 그대로 쓴다
	const bool bHasArgs = InArgs.Num() > 0;
	
	TitleText->SetText(bHasArgs ? FText::Format(InRow.Title, InArgs) : InRow.Title);
	ContentText->SetText(bHasArgs ? FText::Format(InRow.Content, InArgs) : InRow.Content);
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

FReply UPopupWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 버튼 위에서 누르면 버튼이 먼저 처리해서 여기까지 오지 않는다. 여기 온 것은 버튼이 아닌 곳을 누른 경우다.
	if (PopupSizeBox->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()) == false)
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	bDragging = true;
	DragStartMousePosition = FVector2D(InMouseEvent.GetScreenSpacePosition());
	DragStartTranslation = PopupSizeBox->GetRenderTransform().Translation;

	// 마우스가 팝업 밖으로 빨리 나가도 버튼을 뗄 때까지 이벤트를 계속 받도록 캡처한다
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UPopupWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging == false)
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	// 마우스 이동량은 화면 픽셀이고 RenderTranslation 은 UI 단위이므로 DPI 배율로 나눈다
	const float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
	const FVector2D MouseDelta = FVector2D(InMouseEvent.GetScreenSpacePosition()) - DragStartMousePosition;
	FVector2D Translation = DragStartTranslation + MouseDelta / Scale;

	// 팝업은 화면 가운데에서 시작하므로, 중심이 화면 밖으로 나가 잃어버리지 않게 제한한다
	const FVector2D HalfViewport = UWidgetLayoutLibrary::GetViewportSize(this) / (2.f * Scale);
	Translation.X = FMath::Clamp(Translation.X, -HalfViewport.X, HalfViewport.X);
	Translation.Y = FMath::Clamp(Translation.Y, -HalfViewport.Y, HalfViewport.Y);

	PopupSizeBox->SetRenderTranslation(Translation);
	return FReply::Handled();
}

FReply UPopupWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging == false || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	bDragging = false;
	return FReply::Handled().ReleaseMouseCapture();
}
