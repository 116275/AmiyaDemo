// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AmiyaDemoGameMode.generated.h"

class AACoinActor;

UCLASS(abstract)
class AAmiyaDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:

	int32 CoinCount = 0;  // 现在捡了几枚了
	int32 ActiveCoin = 0;   // 场上存在几枚
	int32 MaxCoin = 5;   // 场上最多存在几枚
	int32 CoinGoal = 20;  // 目标是捡几枚

	UPROPERTY(EditAnywhere, Category = "Coin")
	TSubclassOf<AACoinActor> CoinClass;

public:
	
	/** Constructor */
	AAmiyaDemoGameMode();

	void AddCoin();

	void SpawnCoin();

};



