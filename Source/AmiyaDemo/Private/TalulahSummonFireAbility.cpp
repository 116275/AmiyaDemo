#include "TalulahSummonFireAbility.h"
#include "TalulahCharacter.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "CombatAttributeSet.h"
#include "Engine/OverlapResult.h"

UTalulahSummonFireAbility::UTalulahSummonFireAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UTalulahSummonFireAbility::ActivateAbility(   // 负责技能生命周期
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	ATalulahCharacter* Talulah =
		Cast<ATalulahCharacter>(GetAvatarActorFromActorInfo());

	if (!IsValid(Talulah) || Talulah->IsDead())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FVector FootLocation = Talulah->GetActorLocation();
	FootLocation.Z -= Talulah->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector SkillCenter = FootLocation + Talulah->GetActorForwardVector() * CastDistance;

	DrawDebugSphere(
		GetWorld(),
		SkillCenter,
		30.f,
		16,
		FColor::Red,
		false,
		3.f
	);

	// 初版特效

	DrawDebugSphere(
		Talulah->GetWorld(),
		SkillCenter,
		OuterRadius,
		32,
		FColor::Yellow,
		false,
		3.f
	);

	DrawDebugSphere(
		Talulah->GetWorld(),
		SkillCenter,
		InnerRadius,
		32,
		FColor::Red,
		false,
		3.f
	);

	// 初版特效

	ApplyAreaDamage(Talulah, SkillCenter);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTalulahSummonFireAbility::ApplyAreaDamage(ATalulahCharacter* Talulah, const FVector& SkillCenter)
{
	if (!IsValid(Talulah) || !Talulah->HasAuthority())
	{
		return;
	}

	UWorld* World = Talulah->GetWorld();

	UAbilitySystemComponent* SourceASC =
		Talulah->GetAbilitySystemComponent();

	if (!World || !SourceASC || !DamageEffectClass)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Talulah);

	World->OverlapMultiByObjectType(
		OverlapResults,
		SkillCenter,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(OuterRadius),
		QueryParams
	);

	// 进行检测
	TSet<AActor*> HitActors;  // 去重

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Target = Result.GetActor();

		if (!IsValid(Target) || HitActors.Contains(Target))
		{
			continue;
		}

		HitActors.Add(Target);

		IAbilitySystemInterface* TargetASI =
			Cast<IAbilitySystemInterface>(Target);

		if (!TargetASI)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC =
			TargetASI->GetAbilitySystemComponent();

		if (!TargetASC || !TargetASC->HasAttributeSetForAttribute(UCombatAttributeSet::GetHealthAttribute()))
		{
			continue;
		}

		const float Distance = FVector::Dist2D(
			Target->GetActorLocation(),
			SkillCenter
		);

		if (Distance > OuterRadius)
		{
			continue;
		}

		const float FinalDamage =
			Distance <= InnerRadius ? Damage * InnerDamageMultiplier : Damage;

		// 开始施加 GE
		FGameplayEffectSpecHandle SpecHandle =
			MakeOutgoingGameplayEffectSpec(
				DamageEffectClass,
				GetAbilityLevel()
			);

		if (!SpecHandle.IsValid())
		{
			continue;
		}

		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(
				FName(TEXT("Data.Damage"))
			),
			-FinalDamage
		);

		SourceASC->ApplyGameplayEffectSpecToTarget(
			*SpecHandle.Data.Get(),
			TargetASC
		);
		// 板子

	}
}