// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/PopupTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UISubsystem.generated.h"

class UDataTable;
class ULoadingUI;
struct FStreamableHandle;

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

	// 화면을 덮는 로딩 UI 를 띄우고 내린다. 이미 떠 있을 때의 ShowLoading 은 무시한다.
	void ShowLoading();
	void HideLoading();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> PopupTable;

	// Initialize 에서 미리 로드해 둔 팝업 위젯 클래스. UPROPERTY 여야 GC 에 수거되지 않는다.
	UPROPERTY()
	TObjectPtr<UClass> CachedPopupClass;
	
	TSharedPtr<FStreamableHandle> PopupClassLoadHandle;

	// 지금 화면에 떠 있는 로딩 UI. 내릴 때 비운다 (레벨이 바뀌면 소유 플레이어가 달라지므로 재사용하지 않는다).
	UPROPERTY()
	TObjectPtr<ULoadingUI> LoadingWidget;

private:
	const FPopupTypeRow* FindPopupType(FName InRowName) const;
	void HandlePopupClassLoaded();
};
