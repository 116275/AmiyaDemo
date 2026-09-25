// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmiyaDemoGameMode.h"
#include "NavigationSystem.h"
#include "ACoinActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "EnemySpawnVolume.h"
#include "TimerManager.h"

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

		// 5.0s后补充敌人
		StartEnemyRefill();
	}
}

void AAmiyaDemoGameMode::StartEnemyRefill()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(EnemyRefillTimer))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(EnemyRefillTimer, this, &AAmiyaDemoGameMode::TryRefillEnemy, EnemyRefillInterval, true);
}

void AAmiyaDemoGameMode::TryRefillEnemy()
{
	const int RemainingKills = KillGoal - KillCount;
	const int DesiredKills = FMath::Min(MaxActiveEnemies, RemainingKills);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Kills=%d Active=%d Desired=%d"),
				KillCount,
				ActiveEnemies,
				DesiredKills
			)
		);
	}

	if (ActiveEnemies >= DesiredKills)
	{
		GetWorld()->GetTimerManager().ClearTimer(EnemyRefillTimer);
		return;
	}

	SpawnEnemy();
}

bool AAmiyaDemoGameMode::SpawnEnemy()  // 整张地图范围刷新但不在玩家周围
{

	if (!EnemyClass)
	{
		return false;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	AEnemySpawnVolume* SpawnVolume = Cast<AEnemySpawnVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawnVolume::StaticClass()));

	if (!PlayerPawn || !SpawnVolume)
	{
		return false;
	}

	for (int i = 0; i < MaxEnemySpawnAttempts; i++)
	{
		FVector RandomLocation = SpawnVolume->GetRandomPoint();

		FNavLocation NavLocation;

		bool bFoundLocation = NavSystem->ProjectPointToNavigation(RandomLocation, NavLocation, FVector(100.f, 100.f, 200.f));

		if (!bFoundLocation)
		{
			continue;
		}

		const float DistanceToPlayer = FVector::Dist(NavLocation.Location, PlayerPawn->GetActorLocation());

		if (DistanceToPlayer < SafeEnemySpawnDistance)
		{
			continue;
		}

		const AEnemyCharacter* EnemyDefault = EnemyClass->GetDefaultObject<AEnemyCharacter>();  
		// 通过获取 CDO，可以在没有任何实体真正存在于游戏世界中的情况下，直接读取该蓝图里配置好的默认属性

		const float CapsuleHalfHeight = EnemyDefault->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FVector SpawnLocation = NavLocation.Location;
		SpawnLocation.Z += CapsuleHalfHeight + 5.f;      // 调整生成位置，防止敌人卡在地里

		const FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;

		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		AEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (NewEnemy)
		{
			ActiveEnemies++;

			return true;
		}
	}
	return false;
}