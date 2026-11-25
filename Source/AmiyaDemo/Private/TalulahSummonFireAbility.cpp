#include "TalulahSummonFireAbility.h"
#include "TalulahCharacter.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UTalulahSummonFireAbility::UTalulahSummonFireAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UTalulahSummonFireAbility::ActivateAbility(
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

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}