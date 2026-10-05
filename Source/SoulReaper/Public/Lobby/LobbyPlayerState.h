// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

/**
 *
 */
UCLASS()
class SOULREAPER_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// 서버에서만 유효한 값. 클라는 ALobbyGameState::GetLobbyPlayers() 를 사용한다.
	bool IsReady() const { return bIsReady; }
	void SetIsReady(bool bInIsReady);

private:
	bool bIsReady = false;
};
