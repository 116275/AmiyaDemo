// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AmiyaDemoGameMode.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AmiyaDemoCharacter.h"
#include "ThunderstormDamageType.h"
#include "Engine/DamageEvents.h"

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
	if (bIsDead)
	{
		return 0.f;
	}

	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageApplied <= 0.f)
	{
		return 0.f;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageApplied, 0.f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath(EventInstigator, DamageCauser);
	}
	else if (DamageEvent.DamageTypeClass && DamageEvent.DamageTypeClass->IsChildOf(UThunderstormDamageType::StaticClass()))
	{
		ApplyParalysis();
	}

	return DamageApplied;
}

void AEnemyCharacter::HandleDeath(AController* KillerController, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (AAmiyaDemoGameMode* CurrentGameMode = Cast<AAmiyaDemoGameMode>(GetWorld()->GetAuthGameMode()))
	{
		CurrentGameMode->AddKill();
	}

	// 停止角色移动
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// AI 不再控制尸体
	DetachFromControllerPendingDestroy();

	// Capsule 不再挡路
	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	// Skeletal Mesh 交给物理系统
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);  // 尸体不挡子弹

	// 尸体保留一会，再真正销毁 Actor
	SetLifeSpan(DeathDestroyDelay);

	if (KillerController)
	{
		if (AAmiyaDemoCharacter* Killer = Cast<AAmiyaDemoCharacter>(KillerController->GetPawn()))
		{
			Killer->RewardEnergyForKill(1);
		}
	}
}

void AEnemyCharacter::FireAt(AActor* Target)
{
	if (!Target || bIsDead || bIsParalyzed)
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
		AActor* HitActor = HitResult.GetActor();

		if (HitActor->IsA<AEnemyCharacter>())
		{
			return;
		}

		UGameplayStatics::ApplyDamage(HitResult.GetActor(), Damage, GetController(), this, UDamageType::StaticClass());
	}
}

void AEnemyCharacter::SetChainLightningTargetMaterial(bool bIsTarget)
{
	if (bIsTarget)
	{
		GetMesh()->SetOverlayMaterial(TargetMaterial);
	}
	else
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}

void AEnemyCharacter::ApplyParalysis()
{
	if (bIsDead)
	{
		return;
	}

	bIsParalyzed = true;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	if (AController* EnemyController = GetController())
	{
		EnemyController->StopMovement();
	}

	GetWorld()->GetTimerManager().SetTimer(ParalysisTimer, this, &AEnemyCharacter::EndParalysis, ParalysisDuration, false);
}

void AEnemyCharacter::EndParalysis()
{
	if (bIsDead)
	{
		return;
	}

	bIsParalyzed = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
