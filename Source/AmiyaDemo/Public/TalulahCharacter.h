// Fill out your copyright notice in the Description page of Project Settings.
// 鼠标右键一技能，R二技能，T大招

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "TalulahCharacter.generated.h"

class UTalulahAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UCombatAttributeSet;
struct FOnAttributeChangeData;
class UMovementAttributeSet;

UCLASS()
class AMIYADEMO_API ATalulahCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ATalulahCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void HandleDeath();

	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Talulah|Death")
	void OnDeathVisuals();

protected:

	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//virtual void PawnClientRestart();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleDeath();

private:

	bool bIsDead = false;

	void InitAbilityActorInfo();

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UTalulahAttributeSet> TalulahAS;

	UPROPERTY()
	TObjectPtr<UCombatAttributeSet> CombatAS;

	void OnHealthChanged(const FOnAttributeChangeData& Data);

	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(EditAnywhere, Category = "GAS|Test")
	TSubclassOf<UGameplayAbility> TestAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
	TObjectPtr<UInputAction> AttackAction;

	void InputAttack();

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	void InputMove(const FInputActionValue& Value);
	void InputLook(const FInputActionValue& Value);

	void StartSprint();
	void StopSprint();

	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

	UFUNCTION(BlueprintCallable)
	void PerformAttackTrace();

	UPROPERTY(EditDefaultsOnly, Category = "GE|Combat")
	TSubclassOf<UGameplayEffect> BasicAttackDamageEffectClass;

	UPROPERTY()
	TObjectPtr<UMovementAttributeSet> MovementAS;

	UPROPERTY(EditDefaultsOnly, Category = "GE|Movement")
	TSubclassOf<UGameplayEffect> SprintEffectClass;

	FActiveGameplayEffectHandle SprintEffectHandle;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawAttackDebug(FVector Start, FVector End, float Radius, bool bHit);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Skill1Action;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> Skill1AbilityClass;

	void InputSkill1();   // "唤火"

};
