// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RequestHandler.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URequestHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULREAPER_API IRequestHandler
{
	GENERATED_BODY()

public:
	virtual void HandleRequest(int32 InPayload) = 0;
};
