// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PopupTypes.generated.h"

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
};

// 팝업의 좌/우 버튼을 눌렀을 때 실행할 함수 (인자 없음)
DECLARE_DELEGATE(FOnPopupButtonClicked);
