// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
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
	TObjectPtr<UTalulahAttributeSet> AS;

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

	void InputMove(const FInputActionValue& Value);
	void InputLook(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void PerformAttackTrace();

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Combat")
	TSubclassOf<UGameplayEffect> BasicAttackDamageEffectClass;

};
