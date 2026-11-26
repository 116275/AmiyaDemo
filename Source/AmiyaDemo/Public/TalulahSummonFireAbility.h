// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TalulahSummonFireAbility.generated.h"

class ATalulahCharacter;
class UGameplayEffect;

UCLASS()
class AMIYADEMO_API UTalulahSummonFireAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UTalulahSummonFireAbility();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "GA|SummonFire")
	float CastDistance = 600.f;

	virtual void ActivateAbility(						// 负责技能生命周期
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	UPROPERTY(EditDefaultsOnly, Category = "GA|SummonFire")
	float OuterRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "GA|SummonFire")
	float InnerRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "GE|Damage")
	float Damage = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "GE|Damage")
	float InnerDamageMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "GE|Damage")
	float ElementalDamageRatio = 0.75f;  // 后期实现的元素损伤机制

	UPROPERTY(EditDefaultsOnly, Category = "GE|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	void ApplyAreaDamage(ATalulahCharacter* Talulah, const FVector& SkillCenter);  // 专门释放范围伤害
	
};
