// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BossCharacter.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1,
	Phase2
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnTimeDeletionChanged,
	bool,
	IsActive
);

/**
 * 
 */
UCLASS()
class AMIYADEMO_API ABossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
	
public:

	ABossCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Boss|TimeDeletion")
	FOnTimeDeletionChanged OnTimeDeletionChanged;

protected:

	virtual void BeginPlay() override;

	virtual void HandleDeath(
		AController* KillerController,
		AActor* DamageCauser
	)override;

	virtual float GetParalysisDurationScale() const override;

	virtual void AttackTarget(AActor* Target) override;

	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

private:

	UPROPERTY(EditAnywhere, Category = "Boss")
	EBossPhase CurrentPhase = EBossPhase::Phase1;

	UPROPERTY(EditAnywhere, Category = "Boss|Health")
	float Phase1MaxHealth = 500.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Health")
	float Phase2MaxHealth = 1000.f;

	void EnterPhase2();

	UPROPERTY(EditAnywhere, Category = "Boss|Combat")
	UAnimMontage* PunchMontage;

	UPROPERTY(EditAnywhere, Category = "Boss|Combat")
	float PunchDamage = 100.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Combat")
	float PunchRange = 250.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Combat")
	float PunchRadius = 100.f;

	bool bIsAttacking = false;   // 防止动画卡在前半段(重复播放)

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void PerformPunchHit();

	UFUNCTION()
	void OnPunchMontageEnded(
		UAnimMontage* Montage,
		bool bInterrupted
	);

	UPROPERTY(EditAnywhere, Category = "Boss|Combat")
	float Phase2MoveSpeed = 1000.f;

	void IgnoreMovementCollisionWithEnemy(AEnemyCharacter* Enemy);

	UPROPERTY(EditAnywhere, Category = "Boss|Summon")
	TSubclassOf<AEnemyCharacter> MinionClass;

	UPROPERTY(EditAnywhere, Category = "Boss|Summon")
	float SummonInterval = 6.5f;

	UPROPERTY(EditAnywhere, Category = "Boss|Summon")
	float SummonRadius = 600.f;

	FTimerHandle SummonTimer;

	TArray<TWeakObjectPtr<AEnemyCharacter>> CurrentMinions;

	void StartSummoning();
	void SummonMinions();

	int32 GetSummonCount() const
	{
		return CurrentPhase == EBossPhase::Phase1 ? 2 : 4;
	}

	UPROPERTY(EditAnywhere, Category = "Boss|Summon")
	float AbsorbDelay = 4.f;

	FTimerHandle AbsorbTimer;

	void AbsorbRemainingMinions();

	UPROPERTY(EditAnywhere, Category = "Boss|Absorb")
	float AbsorbAttackBonus = 25.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Absorb")
	float AbsorbHealPercent = 0.18f;

	bool bIsTimeDeleting = false;

	UPROPERTY(EditAnywhere, Category = "Boss|TimeDeletion")
	float TimeDeletionDuration = 0.75f;   // 放慢前

	UPROPERTY(EditAnywhere, Category = "Boss|TimeDeletion")
	float GlobalTimeScale = 0.25f;

	FTimerHandle TimeDeletionTimer;

	void StartTimeDeletion();
	void EndTimeDeletion();

	UPROPERTY(EditAnywhere, Category = "Boss|TimeDeletion")
	float TimeDeletionInitialDelay = 2.f;

	UPROPERTY(EditAnywhere, Category = "Boss|TimeDeletion")
	float TimeDeletionCooldown = 5.f;

	FTimerHandle TimeDeletionCooldownTimer;

	void InitialTimeDeletion(float Delay);
	void TryStartTimeDeletion();

	virtual bool IsTargetable() const override;

	ECollisionResponse SavedCapsuleVisibilityResponse;
	ECollisionResponse SavedMeshVisibilityResponse;

	ECollisionResponse SavedPawnCollisionResponse;

	ECollisionResponse SavedCapsuleCameraResponse;
	ECollisionResponse SavedMeshCameraResponse;

protected:

	virtual float GetAttackDistance() const override;
	virtual float GetResumeChaseDistance() const override;
	virtual float GetAttackInterval() const override;
	float GetMoveAcceptanceRadius() const override;
	virtual void CancelCurrentAttack() override;

};
