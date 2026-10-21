// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/PopupTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UISubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SOULREAPER_API UUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// RowName 에 해당하는 팝업을 내용까지 채워서 화면에 띄운다.
	// 좌/우 버튼을 눌렀을 때 실행할 함수를 넘긴다. 쓰지 않는 쪽은 생략하면 닫기만 한다.
	void ShowPopup(FName InRowName,
		FOnPopupButtonClicked InOnLeft = FOnPopupButtonClicked(),
		FOnPopupButtonClicked InOnRight = FOnPopupButtonClicked());

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> PopupTable;
	
private:
	const FPopupTypeRow* FindPopupType(FName InRowName) const;
};
