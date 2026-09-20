// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmiyaDemoGameMode.h"
#include "ACoinActor.h"

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
	CoinCount++;
	ActiveCoin--;
	
	if (CoinCount >= CoinGoal)
	{
		// Handle goal reached
	}
	else
	{
		SpawnCoin();
	}

}

void AAmiyaDemoGameMode::SpawnCoin()  // 补充一枚金币
{
	if (ActiveCoin < MaxCoin && CoinClass)
	{
		FVector SpawnLocation = FVector(FMath::RandRange(-1000.f, 1000.f), FMath::RandRange(-1000.f, 1000.f), 100.f);
		FRotator SpawnRotation = FRotator::ZeroRotator;
		AACoinActor* NewCoin = GetWorld()->SpawnActor<AACoinActor>(CoinClass, SpawnLocation, SpawnRotation);
		if (NewCoin)
		{
			ActiveCoin++;
		}
	}
}