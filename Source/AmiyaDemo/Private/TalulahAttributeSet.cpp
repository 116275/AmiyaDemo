// Fill out your copyright notice in the Description page of Project Settings.


#include "TalulahAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "TalulahCharacter.h"

UTalulahAttributeSet::UTalulahAttributeSet()
{
	InitMaxHealth(100.f);
	InitHealth(100.f);
}

void UTalulahAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTalulahAttributeSet, Health, OldHealth);
}

void UTalulahAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTalulahAttributeSet, MaxHealth, OldMaxHealth);
}

void UTalulahAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTalulahAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTalulahAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UTalulahAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(
			FMath::Clamp(
				GetHealth(),
				0.f,
				GetMaxHealth()
			)
		);

		if (GetHealth() <= 0.f)
		{
			if (ATalulahCharacter* Talulah =
				Cast<ATalulahCharacter>(GetOwningActor()))
			{
				Talulah->HandleDeath();
			}
		}
	}
}

