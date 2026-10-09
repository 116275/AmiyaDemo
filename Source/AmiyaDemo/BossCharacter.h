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

private:

	UPROPERTY(EditAnywhere, Category = "Boss")
	EBossPhase CurrentPhase = EBossPhase::Phase1;

	UPROPERTY(EditAnywhere, Category = "Boss|Health")
	float Phase1MaxHealth = 500.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Health")
	float Phase2MaxHealth = 1000.f;

	void EnterPhase2();

};
