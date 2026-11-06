// Fill out your copyright notice in the Description page of Project Settings.


#include "ATargetDummy.h"

// Sets default values
AATargetDummy::AATargetDummy()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AATargetDummy::BeginPlay()
{
	Super::BeginPlay();
}

float AATargetDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.f)
	{
		Destroy();
	}
	return ActualDamage;
}

