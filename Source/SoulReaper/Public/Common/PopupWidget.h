// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Common/PopupTypes.h"
#include "PopupWidget.generated.h"

class UButton;
class USizeBox;
class UTextBlock;
class UUISubsystem;
/**
 *
 */
UCLASS()
class SOULREAPER_API UPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// InArgs 는 행의 Title/Content 안의 {이름} 자리를 채운다 (예: (ErrorCode: {ErrorCode}))
	void SetData(const FPopupTypeRow& InRow, FOnPopupButtonClicked InOnLeft, FOnPopupButtonClicked InOnRight,
		const FFormatNamedArguments& InArgs = FFormatNamedArguments());

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RightButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ContentText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LeftButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RightButtonText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> PopupSizeBox;

	// 확인만 있는 팝업일 때 오른쪽 버튼 전체(크기 박스 포함)를 숨기기 위한 컨테이너
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> RightSizeBox;

private:
	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

	// 좌/우 버튼을 눌렀을 때 실행할 함수 (호출한 쪽이 정해서 넘겨 줌)
	FOnPopupButtonClicked OnLeft;
	FOnPopupButtonClicked OnRight;

	// 버튼을 누른 뒤 이어서 띄울 팝업의 행 이름 (NAME_None 이면 이어지는 팝업이 없다)
	FName LeftNextPopupRow = NAME_None;
	FName RightNextPopupRow = NAME_None;

	// 버튼을 누르면 이 팝업을 닫을지 (false 면 남는다. 이어지는 팝업 아래에 쌓일 때 쓴다)
	bool bCloseOnLeftClick = true;
	bool bCloseOnRightClick = true;

	// 팝업 본체(PopupSizeBox)를 마우스로 끌어 옮기는 중인지, 그리고 끌기를 시작한 시점의 값
	bool bDragging = false;
	FVector2D DragStartMousePosition = FVector2D::ZeroVector;
	FVector2D DragStartTranslation = FVector2D::ZeroVector;

	UFUNCTION()
	void HandleLeftClicked();

	UFUNCTION()
	void HandleRightClicked();

	void HandleButtonClicked(FOnPopupButtonClicked Callback, bool bClose, FName NextPopupRow);
};
