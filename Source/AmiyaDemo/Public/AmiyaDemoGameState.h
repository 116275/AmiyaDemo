// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AmiyaDemoGameState.generated.h"

class ABossCharacter;

UENUM(BlueprintType)
enum class EGameResult : uint8
{
	None,
	Victory,
	Defeat
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpawned, ABossCharacter*, Boss);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResultChanged, EGameResult, Result);

/**
 * 
 */
UCLASS()
class AMIYADEMO_API AAmiyaDemoGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(Replicated)
	int32 CoinCount = 0;

	UPROPERTY(Replicated)
	int32 CoinGoal = 5;

	UPROPERTY(Replicated)
	int32 KillCount = 0;

	UPROPERTY(Replicated)
	int32 KillGoal = 5;

	UFUNCTION(BlueprintPure)
	int32 GetCoinCount() const { return CoinCount; }

	UFUNCTION(BlueprintPure)
	int32 GetCoinGoal() const { return CoinGoal; }

	UFUNCTION(BlueprintPure)
	int32 GetKillCount() const { return KillCount; }

	UFUNCTION(BlueprintPure)
	int32 GetKillGoal() const { return KillGoal; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 给服务端使用的修改函数
	void SetCoinCount(int32 NewValue);
	void SetKillCount(int32 NewValue);
	void SetGoals(int32 NewCoinGoal, int32 NewKillGoal);

	UPROPERTY(ReplicatedUsing = OnRep_CurrentBoss)
	ABossCharacter* CurrentBoss = nullptr;

	UFUNCTION()
	void OnRep_CurrentBoss();

	UFUNCTION(BlueprintPure)
	ABossCharacter* GetCurrentBoss() const
	{
		return CurrentBoss;
	}

	UPROPERTY(BlueprintAssignable)
	FOnBossSpawned OnBossSpawned;

	void SetCurrentBoss(ABossCharacter* NewBoss);

	UPROPERTY(BlueprintAssignable, Category = "GameResult")
	FOnResultChanged OnResultChanged;

	UPROPERTY(ReplicatedUsing = OnRep_GameResult)
	EGameResult GameResult = EGameResult::None;

	UFUNCTION()
	void OnRep_GameResult();

	void SetGameResult(EGameResult NewResult);

};
