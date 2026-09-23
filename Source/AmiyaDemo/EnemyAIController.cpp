// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EnemyCharacter.h"
#include "DrawDebugHelpers.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GetWorld()->GetTimerManager().SetTimer(ChaseUpdateTimer, this, &AEnemyAIController::UpdateChase, ChaseUpdateInterval, true);
}

void AEnemyAIController::UpdateChase()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!GetPawn() || !PlayerPawn)
	{
		return;
	}

	float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

	if (DistanceToPlayer >= ResumeChaseDistance)   // 1000
	{
		StopFire();
		MoveToActor(PlayerPawn, AcceptDistance, false);
	}
	else if (DistanceToPlayer <= StartFireDistance)   // 800
	{
		StopMovement();
		StartFire();
	}
}

void AEnemyAIController::StartFire()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(FireTimer))
	{
		return;
	}

	FireOnce();

	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &AEnemyAIController::FireOnce, FireInterval, true);

}

void AEnemyAIController::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
}

void AEnemyAIController::FireOnce()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());

	if (PlayerPawn && Enemy)
	{
		Enemy->FireAt(PlayerPawn);
	}
}
