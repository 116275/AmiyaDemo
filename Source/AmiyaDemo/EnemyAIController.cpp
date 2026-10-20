// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EnemyCharacter.h"
#include "DrawDebugHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CombatState = EEnemyCombatState::Chasing;  // 明确初始化状态

	GetWorld()->GetTimerManager().SetTimer(ChaseUpdateTimer, this, &AEnemyAIController::UpdateChase, ChaseUpdateInterval, true);
}

void AEnemyAIController::UpdateChase()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!GetPawn() || !PlayerPawn)
	{
		return;
	}

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());

	if (!Enemy || Enemy->IsDead() || Enemy->IsParalyzed())
	{
		return;
	}

	float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());


	if (CombatState == EEnemyCombatState::Chasing)
	{
		if (DistanceToPlayer <= Enemy->GetAttackDistance())
		{
			StopMovement();
			StartAttack();

			CombatState = EEnemyCombatState::Attacking;
		}
		else
		{
			/*MoveToActor(PlayerPawn, AcceptDistance, false);*/

			const EPathFollowingRequestResult::Type MoveResult = MoveToActor(PlayerPawn, Enemy->GetMoveAcceptanceRadius(), false);

			if (MoveResult == EPathFollowingRequestResult::Failed)
			{
				TryMoveNearPlayer(PlayerPawn);
			}
		}
	}
	else if (CombatState == EEnemyCombatState::Attacking)
	{
		if (DistanceToPlayer >= Enemy->GetResumeChaseDistance())
		{
			StopAttack();

			const EPathFollowingRequestResult::Type MoveResult = MoveToActor(PlayerPawn, Enemy->GetMoveAcceptanceRadius(), false);

			if (MoveResult == EPathFollowingRequestResult::Failed)
			{
				TryMoveNearPlayer(PlayerPawn);
			}

			CombatState = EEnemyCombatState::Chasing;
		}
		else
		{
			if (!GetWorld()->GetTimerManager().IsTimerActive(AttackTimer))
			{
				StartAttack();
			}
		}
	}
}

void AEnemyAIController::StartAttack()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(AttackTimer))
	{
		return;
	}

	AttackOnce();
}

void AEnemyAIController::StopAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(AttackTimer);
}

void AEnemyAIController::AttackOnce()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());

	if (!PlayerPawn || !Enemy)
	{
		return;
	}

	if (Enemy->IsDead() || Enemy->IsParalyzed())
	{
		return;
	}

	Enemy->AttackTarget(PlayerPawn);

	GetWorld()->GetTimerManager().SetTimer(
		AttackTimer,
		this,
		&AEnemyAIController::AttackOnce,
		Enemy->GetAttackInterval(),
		false
	);
}

bool AEnemyAIController::TryMoveNearPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || !GetPawn())
	{
		return false;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!NavSystem)
	{
		return false;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const FVector EnemyLocation = GetPawn()->GetActorLocation();

	constexpr int32 NumSamples = 8;

	bool bFoundPoint = false;
	FVector BestLocation;
	float BestDistance = TNumericLimits<float>::Max();  // 设置极限最大值

	for (float SearchRadius : SearchRadii)
	{
		for (int32 i = 0; i < NumSamples; i++)
		{
			const float Angle = 2.f * PI * static_cast<float>(i) / NumSamples;

			FVector Candidate = PlayerLocation + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * SearchRadius;

			Candidate.Z = EnemyLocation.Z;

			FNavLocation NavLocation;

			const bool bProjected = NavSystem->ProjectPointToNavigation(
				Candidate,
				NavLocation,
				FVector(150.f, 150.f, 400.f)
			);

			if (!bProjected)
			{
				continue;
			}

			UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), EnemyLocation, NavLocation.Location);

			if (!Path || !Path->IsValid() || Path->IsPartial())
			{
				continue;
			}

			const float Distance = FVector::Dist2D(NavLocation.Location, PlayerLocation);

			if (Distance < BestDistance)  // 更新候选点
			{
				BestDistance = Distance;
				BestLocation = NavLocation.Location;
				bFoundPoint = true;
			}
		}
	}

	if (!bFoundPoint)
	{
		return false;
	}

	MoveToLocation(BestLocation, ApproachAcceptDistance);

	return true;
}
