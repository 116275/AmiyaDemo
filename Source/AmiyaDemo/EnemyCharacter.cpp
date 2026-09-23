// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	// 不让 Controller 强行决定身体朝向，而让 CharacterMovement 根据实际移动方向旋转角色
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= DamageApplied;
	if (CurrentHealth <= 0.0f)
	{
		// Handle character death
		Destroy();
	}
	return DamageApplied;
}

void AEnemyCharacter::FireAt(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 50.f);

	FVector AimPoint = Target->GetActorLocation() + FVector(0.f, 0.f, 50.f);

	FVector Direction = (AimPoint - Start).GetSafeNormal();

	FVector End = Start + Direction * FireRange;

	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	FVector DebugEnd = bHit ? HitResult.ImpactPoint : End;

	DrawDebugLine(GetWorld(), Start, DebugEnd, bHit ? FColor::Green : FColor::Red, false, 1.f, 0, 2.f);

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.f, 12, FColor::Yellow, false, 1.f);
	}

	if (bHit && HitResult.GetActor())
	{
		UGameplayStatics::ApplyDamage(HitResult.GetActor(), Damage, GetController(), this, UDamageType::StaticClass());
	}
}
