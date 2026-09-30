// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

enum class EEnemyCombatState
{
	Chasing,
	Firing
};

const float SearchRadii[] =
{
	150.f, 300.f, 450.f, 600.f, 800.f
};

UCLASS()
class AMIYADEMO_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
private:
	virtual void OnPossess(APawn* InPawn) override;

	FTimerHandle ChaseUpdateTimer;

	float ChaseUpdateInterval = 0.2f;

	float ResumeChaseDistance = 900.f;

	float StartFireDistance = 700.f;

	float FireInterval = 1.f;

	float AcceptDistance = 400.f;

	FTimerHandle FireTimer;

	EEnemyCombatState CombatState = EEnemyCombatState::Chasing;

	float ApproachAcceptDistance = 50.f;  // ·þÎñÓÚTryMoveNearPlayer

public:

	void StartFire();

	void StopFire();

	void FireOnce();

	void UpdateChase();

	bool TryMoveNearPlayer(APawn* PlayerPawn);

};
