// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMovementAttributeSet::UMovementAttributeSet()
{
	InitMoveSpeed(500.f);
}

void UMovementAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UMovementAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}
