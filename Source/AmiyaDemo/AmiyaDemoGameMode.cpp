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
#include "Engine/TargetPoint.h"
#include "AmiyaDemoGameInstance.h"
#include "AmiyaDemoGameState.h"

AAmiyaDemoGameMode::AAmiyaDemoGameMode()
{
	GameStateClass = AAmiyaDemoGameState::StaticClass();
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

	StartEnemyRefill();
}

void AAmiyaDemoGameMode::AddCoin()  // 拾取到了金币
{
	AAmiyaDemoGameState* GS = GetAmiyaGameState();
	if (!GS)
	{
		return;
	}

	ActiveCoin--;

	if (GS->GetCoinCount() < GS->GetCoinGoal())
	{
		GS->SetCoinCount(GS->GetCoinCount() + 1);
	}

	if (GS->GetCoinCount() < GS->GetCoinGoal())
	{
		SpawnCoin();
	}

	TryStartBossFight();
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
	AAmiyaDemoGameState* GS = GetAmiyaGameState();
	if (!GS)
	{
		return;
	}

	ActiveEnemies--;

	if (GS->GetKillCount() < GS->GetKillGoal())
	{
		GS->SetKillCount(GS->GetKillCount() + 1);
	}

	if (GS->GetKillCount() < GS->GetKillGoal())
	{
		StartEnemyRefill();
	}
	else
	{
		GetWorldTimerManager().ClearTimer(EnemyRefillTimer);
	}

	TryStartBossFight();
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
	AAmiyaDemoGameState* GS = GetAmiyaGameState();
	if (!GS)
	{
		return;
	}

	const int RemainingKills = GS->GetKillGoal() - GS->GetKillCount();

	const int DesiredEnemies = FMath::Min(MaxActiveEnemies, RemainingKills);

	if (ActiveEnemies >= DesiredEnemies)
	{
		GetWorldTimerManager().ClearTimer(EnemyRefillTimer);
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

void AAmiyaDemoGameMode::TryStartBossFight()   // 能不能生成
{
	if (bBossSpawned)
	{
		return;
	}

	AAmiyaDemoGameState* GS = GetAmiyaGameState();
	if (!GS)
	{
		return;
	}

	if (GS->GetCoinCount() < GS->GetCoinGoal() || GS->GetKillCount() < GS->GetKillGoal())
	{
		return;
	}

	SpawnBoss();
}

bool AAmiyaDemoGameMode::SpawnBoss()  // 生成Boss
{
	if (!BossClass)
	{
		return false;
	}

	TArray<AActor*> SpawnPoints;

	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(),
		BossSpawnTag,
		SpawnPoints
	);

	if (SpawnPoints.IsEmpty())
	{
		return false;
	}

	AActor* SpawnPoint = SpawnPoints[0];

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABossCharacter* Boss = GetWorld()->SpawnActor<ABossCharacter>(
		BossClass,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation(),
		Params
	);

	if (!Boss)
	{
		return false;
	}

	bBossSpawned = true;
	CurrentBoss = Boss;

	AAmiyaDemoGameState* GS = GetAmiyaGameState();

	if (GS)
	{
		GS->SetCurrentBoss(Boss);
	}

	return true;
}

UClass* AAmiyaDemoGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (UAmiyaDemoGameInstance* GameInstance = Cast<UAmiyaDemoGameInstance>(GetGameInstance()))
	{
		if (GameInstance->SelectedCharacterClass)
		{
			return GameInstance->SelectedCharacterClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AAmiyaDemoGameState* AAmiyaDemoGameMode::GetAmiyaGameState() const
{
	return GetGameState<AAmiyaDemoGameState>();
}

int32 AAmiyaDemoGameMode::GetCoinCount() const
{
	const AAmiyaDemoGameState* GS = GetGameState<AAmiyaDemoGameState>();

	return GS ? GS->GetCoinCount() : 0;
}

int32 AAmiyaDemoGameMode::GetCoinGoal() const
{
	const AAmiyaDemoGameState* GS = GetGameState<AAmiyaDemoGameState>();

	return GS ? GS->GetCoinGoal() : 0;
}

int32 AAmiyaDemoGameMode::GetKillCount() const
{
	const AAmiyaDemoGameState* GS = GetGameState<AAmiyaDemoGameState>();

	return GS ? GS->GetKillCount() : 0;
}

int32 AAmiyaDemoGameMode::GetKillGoal() const
{
	const AAmiyaDemoGameState* GS = GetGameState<AAmiyaDemoGameState>();

	return GS ? GS->GetKillGoal() : 0;
}