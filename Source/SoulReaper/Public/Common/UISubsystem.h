// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UISubsystem.generated.h"

struct FPopupTypeRow;
/**
 * 
 */
UCLASS()
class SOULREAPER_API UUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// RowName 에 해당하는 팝업을 내용까지 채워서 화면에 띄운다
	void ShowPopup(FName InRowName);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> PopupTable;
	
private:
	const FPopupTypeRow* FindPopupType(FName InRowName) const;
};
