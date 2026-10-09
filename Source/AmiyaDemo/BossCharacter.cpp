// Fill out your copyright notice in the Description page of Project Settings.


#include "BossCharacter.h"

ABossCharacter::ABossCharacter()
{
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentPhase = EBossPhase::Phase1;

	MaxHealth = Phase1MaxHealth;
	CurrentHealth = MaxHealth;
}

void ABossCharacter::HandleDeath(
	AController* KillerController,
	AActor* DamageCauser
)
{
	if (CurrentPhase == EBossPhase::Phase1)
	{
		EnterPhase2();
		return;
	}

	Super::HandleDeath(KillerController, DamageCauser);
}

void ABossCharacter::EnterPhase2()
{
	CurrentPhase = EBossPhase::Phase2;

	MaxHealth = Phase2MaxHealth;
	CurrentHealth = MaxHealth;

	ClearParalysis();
}

float ABossCharacter::GetParalysisDurationScale() const
{
	if (CurrentPhase == EBossPhase::Phase1)
	{
		return 0.5f;
	}

	return 0.f;
}