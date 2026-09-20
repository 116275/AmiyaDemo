// Fill out your copyright notice in the Description page of Project Settings.


#include "ACoinActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AmiyaDemoCharacter.h"
#include "AmiyaDemoGameMode.h"

// Sets default values
AACoinActor::AACoinActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);

	Collision->InitSphereRadius(60.f);

	Collision->OnComponentBeginOverlap.AddDynamic(this, &AACoinActor::Overlap);

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void AACoinActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AACoinActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AACoinActor::Overlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AAmiyaDemoCharacter>(OtherActor))
	{
		AAmiyaDemoGameMode* GameMode = Cast<AAmiyaDemoGameMode>(GetWorld()->GetAuthGameMode()); // 只在 Authority 侧存在，单机 Demo 现在没事

		if (GameMode)
		{
			GameMode->AddCoin();
		}

		Destroy();
	}
}

