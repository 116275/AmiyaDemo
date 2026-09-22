// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"


UCLASS()
class AMIYADEMO_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
private:
	virtual void OnPossess(APawn* InPawn) override;

	void StartChase();
};
