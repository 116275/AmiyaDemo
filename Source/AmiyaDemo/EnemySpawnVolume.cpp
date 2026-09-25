// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnVolume.h"
#include "Components/BoxComponent.h"

// Sets default values
AEnemySpawnVolume::AEnemySpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SetRootComponent(SpawnBox);

	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called when the game starts or when spawned
void AEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemySpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AEnemySpawnVolume::GetRandomPoint() const
{

	const FVector Center = SpawnBox->GetComponentLocation();
	const FVector Extent = SpawnBox->GetScaledBoxExtent();

	return FVector(FMath::FRandRange(Center.X - Extent.X, Center.X + Extent.X), FMath::FRandRange(Center.Y - Extent.Y, Center.Y + Extent.Y), Center.Z);
}

