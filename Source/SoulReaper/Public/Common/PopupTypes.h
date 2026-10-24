// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PopupTypes.generated.h"

// 버튼을 눌렀을 때 UISubsystem 이 대신 실행해 주는 동작.
// 어디서 띄웠는지와 상관없이 실행할 수 있는 굵직한 동작만 둔다. 호출자의 사정이 필요한 동작(예: 재시도)은 호출자가 델리게이트로 넘긴다.
UENUM(BlueprintType)
enum class EPopupAction : uint8
{
	None			UMETA(DisplayName = "None"),
	GoToMainMenu	UMETA(DisplayName = "Go To Main Menu"),
	QuitGame		UMETA(DisplayName = "Quit Game")
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FPopupTypeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	FText Title;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	FText Content;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	bool bIsConfirmOnly = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	FText LeftText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	FText RightText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	FName LeftNextPopupRow = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	FName RightNextPopupRow = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	bool bCloseOnLeftClick = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	bool bCloseOnRightClick = true;

	// 호출자가 델리게이트를 넘기지 않았을 때만 쓰인다 (호출자가 넘긴 것이 항상 우선)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	EPopupAction LeftAction = EPopupAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulReaper|Widget")
	EPopupAction RightAction = EPopupAction::None;
};

// 팝업의 좌/우 버튼을 눌렀을 때 실행할 함수 (인자 없음)
DECLARE_DELEGATE(FOnPopupButtonClicked);
