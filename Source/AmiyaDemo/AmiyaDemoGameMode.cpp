// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmiyaDemoGameMode.h"
#include "NavigationSystem.h"
#include "ACoinActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"

AAmiyaDemoGameMode::AAmiyaDemoGameMode()
{
	// stub
}

void AAmiyaDemoGameMode::BeginPlay()
{
	Super::BeginPlay();
	for (int i = 0; i < MaxCoin; i++)
	{
		SpawnCoin();
	}
	for (int i = 0; i < MaxActiveEnemies; i++)
	{
		SpawnEnemy();
	}

	DrawDebugSphere(
		GetWorld(),
		EnemySpawnCenter,
		1000.f,
		24,
		FColor::Red,
		true
	);

}

void AAmiyaDemoGameMode::AddCoin()  // 拾取到了金币
{

	ActiveCoin--;

	if (CoinCount >= CoinGoal)
	{
		return;
	}

	CoinCount++;
	
	if (CoinCount >= CoinGoal)
	{
		// Handle goal reached
	}
	else
	{
		SpawnCoin();
	}

}

bool AAmiyaDemoGameMode::SpawnCoin()  // 补充一枚金币
{

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!PlayerPawn || !NavSystem || !CoinClass || ActiveCoin >= MaxCoin)
	{
		return false;
	}

	FNavLocation RandomLocation;
	
	for (int32 i = 0; i < MaxSpawnAttempts; i++) // 尝试MaxSpawnAttempts次寻找随机点
	{

		bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(PlayerPawn->GetActorLocation(), CoinSpawnRadius, RandomLocation);

		if (bFoundLocation)   // 如果找到了一个可达的随机点
		{
			FVector SpawnLocation = RandomLocation.Location;
			SpawnLocation.Z += CoinSpawnHeight; // Adjust the Z position to be above the ground
			FRotator SpawnRotation = FRotator::ZeroRotator;
			AACoinActor* NewCoin = GetWorld()->SpawnActor<AACoinActor>(CoinClass, SpawnLocation, SpawnRotation);  // 目前金币生成在玩家附近
			if (NewCoin)
			{
				ActiveCoin++;
				return true;
			}
		}
	}
	return false;
}

void AAmiyaDemoGameMode::AddKill()
{

	ActiveEnemies--;

	if (KillCount < KillGoal)
	{
		KillCount++;
		SpawnEnemy();
	}
}

bool AAmiyaDemoGameMode::SpawnEnemy()  // 整张地图范围刷新但不在玩家周围
{

	if (!EnemyClass)
	{
		return false;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!PlayerPawn || !NavSystem)
	{
		return false;
	}

	for (int i = 0; i < MaxEnemySpawnAttempts; i++)
	{
		FNavLocation RandomLocation;

		bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(EnemySpawnCenter, EnemySpawnRadius, RandomLocation);

		if (!bFoundLocation)
		{
			continue;
		}

		const float DistanceToPlayer = FVector::Dist(RandomLocation.Location, PlayerPawn->GetActorLocation());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString::Printf(TEXT("Now: %.0f"), DistanceToPlayer));
		}

		if (DistanceToPlayer < SafeEnemySpawnDistance)
		{
			continue;
		}

		///

		const AEnemyCharacter* EnemyDefault =
			EnemyClass->GetDefaultObject<AEnemyCharacter>();

		const float CapsuleHalfHeight =
			EnemyDefault->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		///

		FVector SpawnLocation = RandomLocation.Location;
		SpawnLocation.Z += CapsuleHalfHeight;

		DrawDebugSphere(
			GetWorld(),
			RandomLocation.Location,
			500.f,
			24,
			FColor::Yellow,
			true
		);

		const FRotator SpawnRotation = FRotator::ZeroRotator;

		///

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Yellow,
				FString::Printf(
					TEXT("Spawn: X=%.0f Y=%.0f Z=%.0f"),
					SpawnLocation.X,
					SpawnLocation.Y,
					SpawnLocation.Z
				)
			);

			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Cyan,
				FString::Printf(
					TEXT("Player: X=%.0f Y=%.0f Z=%.0f"),
					PlayerPawn->GetActorLocation().X,
					PlayerPawn->GetActorLocation().Y,
					PlayerPawn->GetActorLocation().Z
				)
			);
		}

		FActorSpawnParameters SpawnParams;

		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		///

		AEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (NewEnemy)
		{
			ActiveEnemies++;

			return true;
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Fail! Again!"));
			}
		}
	}
	return false;
}