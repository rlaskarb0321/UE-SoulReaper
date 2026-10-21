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
};
