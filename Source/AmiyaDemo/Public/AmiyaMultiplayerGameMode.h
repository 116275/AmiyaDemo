// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmiyaDemoGameMode.h"
#include "AmiyaMultiplayerGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMIYADEMO_API AAmiyaMultiplayerGameMode : public AAmiyaDemoGameMode
{
	GENERATED_BODY()
	
public:

	virtual bool AllowPausing(APlayerController* PC) override;

};
