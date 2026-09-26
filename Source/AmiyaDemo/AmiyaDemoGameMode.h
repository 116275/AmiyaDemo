// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnemyCharacter.h"
#include "AmiyaDemoGameMode.generated.h"

class AACoinActor;

UCLASS(abstract)
class AAmiyaDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

private:

	int32 CoinCount = 0;  // 现在捡了几枚了
	int32 ActiveCoin = 0;   // 场上存在几枚
	int32 MaxCoin = 5;   // 场上最多存在几枚
	int32 CoinGoal = 20;  // 目标是捡几枚

	int32 KillCount = 0;
	int32 KillGoal = 10;

public:

	UPROPERTY(EditAnywhere, Category = "Coin")
	int32 MaxSpawnAttempts = 10;

	UPROPERTY(EditAnywhere, Category = "Coin")
	float CoinSpawnRadius = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Coin")
	TSubclassOf<AACoinActor> CoinClass;

	UPROPERTY(EditAnywhere, Category = "Coin")
	float CoinSpawnHeight = 50.f;  // 金币生成的高度

	UFUNCTION(BlueprintPure, Category = "Coin")
	int32 GetCoinCount() const
	{
		return CoinCount;
	}

	UFUNCTION(BlueprintPure, Category = "Coin")
	int32 GetCoinGoal() const
	{
		return CoinGoal;
	}

	UFUNCTION(BlueprintPure, Category = "Kill")
	int32 GetKillCount()
	{
		return KillCount;
	}

	UFUNCTION(BlueprintPure, Category = "Kill")
	int32 GetKillGoal()
	{
		return KillGoal;
	}

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")
	int32 MaxActiveEnemies = 3;

	int32 ActiveEnemies = 0;

	bool SpawnEnemy();

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")
	FVector EnemySpawnCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")  // 足够大以覆盖地图
	float EnemySpawnRadius = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")
	float SafeEnemySpawnDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")
	int32 MaxEnemySpawnAttempts = 30;

	FTimerHandle EnemyRefillTimer;

	UPROPERTY(EditAnywhere, Category = "Enemy Spawn")
	float EnemyRefillInterval = 3.5f;

public:
	
	/** Constructor */
	AAmiyaDemoGameMode();

	void AddCoin();

	bool SpawnCoin();

	void AddKill();

	void StartEnemyRefill();

	void TryRefillEnemy();

};