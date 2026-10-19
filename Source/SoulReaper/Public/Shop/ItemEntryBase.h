// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemEntryBase.generated.h"

class UImage;
class UTexture2D;
struct FShopItemRow;
class UButton;
/**
 *
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemEntryClicked, FName RowName);

UCLASS()
class SOULREAPER_API UItemEntryBase : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnItemEntryClicked OnItemEntryClicked;

public:
	FName GetItemName() const { return RowName; }

	virtual void ClearEntry();
	virtual void Init(FName InRowName, const FShopItemRow& InItemRow);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SlotButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	FName RowName = NAME_None;

	virtual void NativeOnInitialized() override;

private:
	// 이 칸이 "지금" 보여 줘야 하는 아이콘. 비동기 로드 콜백은 이 값만 믿는다.
	TSoftObjectPtr<UTexture2D> ExpectedIcon;

	UFUNCTION()
	void HandleOnItemEntryClicked();

	void HandleIconLoaded();
	void ApplyIcon(UTexture2D* InTexture);
};
