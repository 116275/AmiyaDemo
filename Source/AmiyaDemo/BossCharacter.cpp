// Fill out your copyright notice in the Description page of Project Settings.


#include "BossCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "AmiyaDemoGameMode.h"

ABossCharacter::ABossCharacter()
{
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	StartSummoning();

	CurrentPhase = EBossPhase::Phase1;

	MaxHealth = Phase1MaxHealth;
	CurrentHealth = MaxHealth;

	TArray<AActor*> Enemies;

	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AEnemyCharacter::StaticClass(),
		Enemies
	);

	for (AActor* Actor : Enemies)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);

		if (Enemy && Enemy != this)
		{
			IgnoreMovementCollisionWithEnemy(Enemy);
		}
	}

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

	if (AAmiyaDemoGameMode* GameMode = Cast<AAmiyaDemoGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->FinishGame(EGameResult::Victory);
	}
}

void ABossCharacter::EnterPhase2()
{
	CurrentPhase = EBossPhase::Phase2;

	MaxHealth = Phase2MaxHealth;
	CurrentHealth = MaxHealth;

	ClearParalysis();

	GetCharacterMovement()->MaxWalkSpeed = Phase2MoveSpeed;

	InitialTimeDeletion(TimeDeletionInitialDelay);
}

float ABossCharacter::GetParalysisDurationScale() const
{
	if (CurrentPhase == EBossPhase::Phase1)
	{
		return 0.5f;
	}

	return 0.f;
}

void ABossCharacter::AttackTarget(AActor* Target)
{
	if (!Target || IsDead() || IsParalyzed() || bIsAttacking || bIsTimeDeleting)
	{
		return;
	}

	if (!PunchMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (!AnimInstance)
	{
		return;
	}

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();

	ToTarget.Z = 0.f;

	if (!ToTarget.IsNearlyZero())  // 防止除以0错误
	{
		const FRotator TargetRotation = ToTarget.Rotation();

		SetActorRotation(
			FRotator(
				0.f,
				TargetRotation.Yaw,
				0.f
			)
		);
	}

	const float Duration = AnimInstance->Montage_Play(PunchMontage);

	if (Duration <= 0.f)
	{
		return;
	}

	bIsAttacking = true;

	FOnMontageEnded EndDelegate;

	EndDelegate.BindUObject(
		this,
		&ABossCharacter::OnPunchMontageEnded
	);

	AnimInstance->Montage_SetEndDelegate(
		EndDelegate,
		PunchMontage   // 当且仅当 PunchMontage 播放结束或被打断时才触发刚刚绑定的委托
	);
}

void ABossCharacter::OnPunchMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

float ABossCharacter::GetAttackDistance() const
{
	return 212.f;
}

float ABossCharacter::GetResumeChaseDistance() const
{
	return 300.f;
}

float ABossCharacter::GetAttackInterval() const
{
	return CurrentPhase == EBossPhase::Phase1
		? 1.6f
		: 0.5f;
}

float ABossCharacter::GetMoveAcceptanceRadius() const
{
	return 50.f;
}

void ABossCharacter::PerformPunchHit()
{
	if (IsDead() || bIsTimeDeleting || IsParalyzed())
	{
		return;
	}

	const FVector Forward = GetActorForwardVector();

	const FVector Start = GetActorLocation() + Forward * 100.f;

	const FVector End = Start + Forward * PunchRange;

	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const FCollisionShape Sphere = FCollisionShape::MakeSphere(PunchRadius);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		Sphere,
		Params
	);

	if (!bHit || !HitResult.GetActor())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		HitResult.GetActor(),
		PunchDamage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
}

void ABossCharacter::IgnoreMovementCollisionWithEnemy(AEnemyCharacter* Enemy)
{
	if (!Enemy || Enemy == this)
	{
		return;
	}

	GetCapsuleComponent()->IgnoreActorWhenMoving(Enemy, true);
	Enemy->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
}

void ABossCharacter::SummonMinions()
{
	if (!MinionClass || IsDead())
	{
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!NavSystem)
	{
		return;
	}

	const int32 Count = GetSummonCount();
	int32 CurrentCount = 0;

	for (int32 i = 1; i <= Count * 2 && CurrentCount < Count; ++i)
	{
		FNavLocation SpawnLocation;

		const bool bFoundLocation =
			NavSystem->GetRandomReachablePointInRadius(
				GetActorLocation(),
				SummonRadius,
				SpawnLocation
			);

		if (!bFoundLocation)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;

		AEnemyCharacter* SpawnedEnemy =
			GetWorld()->SpawnActor<AEnemyCharacter>(
				MinionClass,
				SpawnLocation.Location,
				FRotator::ZeroRotator,
				SpawnParams
			);

		if (!SpawnedEnemy)
		{
			continue;
		}

		SpawnedEnemy->SetCountsTowardKillGoal(false);

		CurrentMinions.Add(SpawnedEnemy);

		IgnoreMovementCollisionWithEnemy(SpawnedEnemy);

		++CurrentCount;
	}

	GetWorld()->GetTimerManager().SetTimer(
		AbsorbTimer,
		this,
		&ABossCharacter::AbsorbRemainingMinions,
		AbsorbDelay,
		false
	);
}

void ABossCharacter::StartSummoning()
{
	SummonMinions();

	GetWorld()->GetTimerManager().SetTimer(
		SummonTimer,
		this,
		&ABossCharacter::SummonMinions,
		SummonInterval,
		true
	);
}

void ABossCharacter::AbsorbRemainingMinions()
{
	int32 ActiveCount = 0;

	for (const TWeakObjectPtr<AEnemyCharacter>& MinionPtr : CurrentMinions)
	{
		if (!MinionPtr.IsValid())
		{
			continue;
		}

		AEnemyCharacter* Minion = MinionPtr.Get();

		if (!Minion || Minion->IsDead())
		{
			continue;
		}

		Minion->BeConsumedByBoss();
		++ActiveCount;
	}

	if (ActiveCount > 0)
	{
		PunchDamage += AbsorbAttackBonus * ActiveCount;

		const float HealAmount =
			MaxHealth *
			AbsorbHealPercent *
			ActiveCount;

		CurrentHealth = FMath::Clamp(
			CurrentHealth + HealAmount,
			0.f,
			MaxHealth
		);
	}

	CurrentMinions.Empty();
}

void ABossCharacter::StartTimeDeletion()
{
	if (bIsTimeDeleting || IsDead() || CurrentPhase != EBossPhase::Phase2)
	{
		return;
	}

	bIsTimeDeleting = true;

	OnTimeDeletionChanged.Broadcast(true);

	UGameplayStatics::SetGlobalTimeDilation(
		GetWorld(),
		GlobalTimeScale
	);

	CustomTimeDilation = 1.f / GlobalTimeScale;

	SetActorHiddenInGame(true);  // 不可见

	SavedCapsuleVisibilityResponse = GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Visibility);

	SavedMeshVisibilityResponse = GetMesh()->GetCollisionResponseToChannel(ECC_Visibility);

	GetCapsuleComponent()->SetCollisionResponseToChannel(   // 不可被射线检测
		ECC_Visibility,
		ECR_Ignore
	);

	GetMesh()->SetCollisionResponseToChannel(   // 不可被射线检测
		ECC_Visibility,
		ECR_Ignore
	);

	SavedPawnCollisionResponse = GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	SavedCapsuleCameraResponse = GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Camera);

	SavedMeshCameraResponse = GetMesh()->GetCollisionResponseToChannel(ECC_Camera);

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Camera,
		ECR_Ignore
	);

	GetMesh()->SetCollisionResponseToChannel(
		ECC_Camera,
		ECR_Ignore
	);

	GetWorld()->GetTimerManager().SetTimer(
		TimeDeletionTimer,
		this,
		&ABossCharacter::EndTimeDeletion,
		TimeDeletionDuration,
		false
	);

}

void ABossCharacter::EndTimeDeletion()
{
	if (!bIsTimeDeleting)
	{
		return;
	}

	bIsTimeDeleting = false;

	OnTimeDeletionChanged.Broadcast(false);

	UGameplayStatics::SetGlobalTimeDilation(
		GetWorld(),
		1.f
	);

	CustomTimeDilation = 1.f;

	SetActorHiddenInGame(false);

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Visibility,
		SavedCapsuleVisibilityResponse
	);

	GetMesh()->SetCollisionResponseToChannel(
		ECC_Visibility,
		SavedMeshVisibilityResponse
	);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnCollisionResponse);

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Camera,
		SavedCapsuleCameraResponse
	);

	GetMesh()->SetCollisionResponseToChannel(
		ECC_Camera,
		SavedMeshCameraResponse
	);

	GetWorld()->GetTimerManager().ClearTimer(TimeDeletionTimer);

	// 尝试偷袭
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!PlayerPawn)
	{
		return;
	}

	const float Distance =
		FVector::Dist(
			GetActorLocation(),
			PlayerPawn->GetActorLocation()
		);

	if (Distance <= GetAttackDistance())
	{
		if (AController* BossController = GetController())
		{
			BossController->StopMovement();
		}

		AttackTarget(PlayerPawn);
	}


	if (!IsDead() && CurrentPhase == EBossPhase::Phase2)
	{
		InitialTimeDeletion(TimeDeletionCooldown);
	}
}

void ABossCharacter::InitialTimeDeletion(float Delay)
{
	GetWorld()->GetTimerManager().SetTimer(
		TimeDeletionCooldownTimer,
		this,
		&ABossCharacter::TryStartTimeDeletion,
		Delay,
		false
	);
}

void ABossCharacter::TryStartTimeDeletion()
{
	if (bIsTimeDeleting || IsDead() || CurrentPhase != EBossPhase::Phase2)
	{
		return;
	}

	StartTimeDeletion();
}

bool ABossCharacter::IsTargetable() const
{
	return !IsDead() && !bIsTimeDeleting;
}

float ABossCharacter::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (bIsTimeDeleting)
	{
		return 0.f;
	}

	return Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);
}

void ABossCharacter::CancelCurrentAttack()
{
	if (!PunchMontage)
	{
		bIsAttacking = false;
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(PunchMontage))
		{
			AnimInstance->Montage_Stop(
				0.1f,
				PunchMontage
			);
		}
	}

	bIsAttacking = false;
}