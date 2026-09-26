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

UCLASS()
class AMIYADEMO_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
private:
	virtual void OnPossess(APawn* InPawn) override;

	FTimerHandle ChaseUpdateTimer;

	float ChaseUpdateInterval = 0.2f;

	float ResumeChaseDistance = 1000.f;

	void UpdateChase();

	float StartFireDistance = 800.f;

	void StartFire();

	void StopFire();

	void FireOnce();

	float FireInterval = 1.f;

	float AcceptDistance = 700.f;

	FTimerHandle FireTimer;

	EEnemyCombatState CombatState = EEnemyCombatState::Chasing;

};
