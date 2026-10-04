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

	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.0f;

	float FireRange = 1200.f;

	float FireInterval = 1.f;

	float Damage = 5.f;

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	UMaterialInterface* TargetMaterial;

	bool bIsDead = false;

	UPROPERTY(EditAnywhere, Category = "Death")
	float DeathDestroyDelay = 5.f;

	void HandleDeath(AController* KillerController, AActor* DamageCauser);

public:

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void FireAt(AActor* Target);

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

};
