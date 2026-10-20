// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class AMIYADEMO_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	float FireRange = 1200.f;

	float FireInterval = 1.f;

	float Damage = 5.f;

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	UMaterialInterface* TargetMaterial;

	bool bIsDead = false;

	UPROPERTY(EditAnywhere, Category = "Death")
	float DeathDestroyDelay = 5.f;

	bool bIsParalyzed = false;  // 是否处于麻痹状态

	UPROPERTY(EditAnywhere, Category = "Paralysis")
	float ParalysisDuration = 5.f;

	FTimerHandle ParalysisTimer;

	void ApplyParalysis();
	void EndParalysis();

	bool bCountsTowardKillGoal = true;

public:

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void AttackTarget(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const
	{
		return MaxHealth;
	}

	void SetChainLightningTargetMaterial(bool bIsTarget);

	bool IsDead() const
	{
		return bIsDead;
	}

	UFUNCTION(BlueprintPure, Category = "State")
	virtual bool IsTargetable() const
	{
		return !bIsDead;
	}

	FVector GetBodyLocation() const   // 防止尸体滚动造成CapsuleComponent的中心点偏移，使用Mesh的Bounds.Origin作为身体位置
	{
		if (GetMesh())
		{
			return GetMesh()->Bounds.Origin;
		}

		return GetActorLocation();
	}

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsParalyzed() const
	{
		return bIsParalyzed;
	}

	void SetCountsTowardKillGoal(bool bCounts)
	{
		bCountsTowardKillGoal = bCounts;
	}

protected:

	float CurrentHealth = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 150.0f;

	virtual void HandleDeath(AController* KillerController, AActor* DamageCauser);

	virtual float GetParalysisDurationScale() const   // 方便后面 Boss 加麻痹抗性
	{
		return 1.f;
	}

	void ClearParalysis();

public:

	virtual float GetAttackDistance() const
	{
		return 700.f;
	}

	virtual float GetResumeChaseDistance() const
	{
		return 900.f;
	}

	virtual float GetAttackInterval() const
	{
		return 1.f;
	}

	virtual float GetMoveAcceptanceRadius() const
	{
		return 400.f;
	}

	void BeConsumedByBoss();

};
