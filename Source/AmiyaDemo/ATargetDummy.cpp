// Fill out your copyright notice in the Description page of Project Settings.


#include "ATargetDummy.h"

// Sets default values
AATargetDummy::AATargetDummy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AATargetDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AATargetDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

