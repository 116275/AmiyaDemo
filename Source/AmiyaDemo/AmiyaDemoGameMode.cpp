// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmiyaDemoGameMode.h"
#include "NavigationSystem.h"
#include "ACoinActor.h"
#include "Kismet/GameplayStatics.h"

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