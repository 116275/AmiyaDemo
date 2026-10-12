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

/**
 * 
 */
UCLASS()
class AMIYADEMO_API ABossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
	
public:

	ABossCharacter();

protected:

	virtual void BeginPlay() override;

	virtual void HandleDeath(
		AController* KillerController,
		AActor* DamageCauser
	)override;

	virtual float GetParalysisDurationScale() const override;

	virtual void AttackTarget(AActor* Target) override;

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

	bool bIsAttacking = false;   // ·ÀÖ¹¶¯»­¿¨ÔÚÇ°°ë¶Î

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void PerformPunchHit();

	UFUNCTION()
	void OnPunchMontageEnded(
		UAnimMontage* Montage,
		bool bInterrupted
	);

	virtual float GetAttackDistance() const override;
	virtual float GetResumeChaseDistance() const override;
	virtual float GetAttackInterval() const override;
	float GetMoveAcceptanceRadius() const override;

	UPROPERTY(EditAnywhere, Category = "Boss|Combat")
	float Phase2MoveSpeed = 950.f;

	void IgnoreMovementCollisionWithEnemy(AEnemyCharacter* Enemy);

};
