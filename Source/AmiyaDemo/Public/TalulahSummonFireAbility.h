// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TalulahSummonFireAbility.generated.h"

/**
 * 
 */
UCLASS()
class AMIYADEMO_API UTalulahSummonFireAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UTalulahSummonFireAbility();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "GA|SummonFire")
	float CastDistance = 600.f;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
};
