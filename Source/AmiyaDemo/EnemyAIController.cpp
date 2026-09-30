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

	float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());


	if (CombatState == EEnemyCombatState::Chasing)
	{
		if (DistanceToPlayer <= StartFireDistance)
		{
			StopMovement();
			StartFire();

			CombatState = EEnemyCombatState::Firing;
		}
		else
		{
			/*MoveToActor(PlayerPawn, AcceptDistance, false);*/

			const EPathFollowingRequestResult::Type MoveResult = MoveToActor(PlayerPawn, AcceptDistance, false);

			if (MoveResult == EPathFollowingRequestResult::Failed)
			{
				TryMoveNearPlayer(PlayerPawn);
			}
		}
	}
	else if (CombatState == EEnemyCombatState::Firing)
	{
		if (DistanceToPlayer >= ResumeChaseDistance)
		{
			StopFire();

			const EPathFollowingRequestResult::Type MoveResult = MoveToActor(PlayerPawn, AcceptDistance, false);

			if (MoveResult == EPathFollowingRequestResult::Failed)
			{
				TryMoveNearPlayer(PlayerPawn);
			}

			CombatState = EEnemyCombatState::Chasing;
		}
		else
		{
			if (!GetWorld()->GetTimerManager().IsTimerActive(FireTimer))
			{
				StartFire();
			}
		}
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

			if (!NavSystem->ProjectPointToNavigation(Candidate, NavLocation, FVector(150.f, 150.f, 200.f)))
			{
				continue;
			}

			UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), EnemyLocation, NavLocation.Location);

			if (!Path || !Path->IsValid() || Path->IsPartial())
			{
				continue;
			}

			const float Distance = FVector::Dist(NavLocation.Location, PlayerLocation);

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
