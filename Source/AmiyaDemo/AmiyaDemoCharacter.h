// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Camera/CameraComponent.h"
#include "AmiyaDemoCharacter.generated.h"


UENUM(BlueprintType)
enum class ESkillPowerState : uint8
{
	Base,
	Enhanced,
	Overcharged
};

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UAnimMontage;
class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnergyChanged, int32, NewEnergy);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AAmiyaDemoCharacter : public ACharacter
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:  //暴露给编辑器的输入相关属性

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ParryAction;

	UPROPERTY(EditAnywhere, Category = "Move")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float SprintSpeed = 800.f;

	void StartSprint();

	void StopSprint();

public:

	/** Constructor */
	AAmiyaDemoCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Attack();

	virtual void Tick(float Delta) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	// 为了读取Private变量提供两个只读接口，BlueprintPure表示不改变对象状态
	UFUNCTION(BlueprintPure, Category = "Health") float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health") float GetMaxHealth() const { return MaxHealth; }

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:  // 自定义参数

	float CurrentHealth = 100.f;
	float MaxHealth = 300.f;

	float StartOffset = 50.f;
	float AttackRange = 100.f;
	float Radius = 50.f;
	float Damage = 50.f;

	float RegenTime = 3.f;   // 多少秒后开始回血
	float RegenAmount = 10.f;   // 每秒回血量
	float TimeSinceLastDamage = 0.f;  // 离上一次受伤隔了多久

	UPROPERTY(EditAnywhere, Category = "Energy")
	int32 MaxEnergy = 8;

	int32 CurrentEnergy = 0;
	int32 EnergyRegen = 1;

	UPROPERTY(EditAnywhere, Category = "Energy")
	float EnergyRegenInterval = 5.f;

	FTimerHandle EnergyRegenTimer;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryDuration = 2.f;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float PerfectParryDuration = 0.2f;

	bool bIsParrying = false;
	bool bIsPerfectParrying = false;

	FTimerHandle ParryTimer;
	FTimerHandle PerfectParryTimer;

	void EndParry();
	void StartParryState();
	void EndParryState();
	void EndPerfectParry();

	UPROPERTY(EditAnywhere, Category = "Parry Feedback")
	float PerfectParryFOV = 65.f;

	UPROPERTY(EditAnywhere, Category = "Parry Feedback")
	float PerfectParrySlowFactor = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Parry Feedback")
	float HighLightDuration = 1.f;

	float DefaultFOV = 0.f;

	FTimerHandle PerfectParryFeedbackTimer;

	void StartPerfectParryFeedback();
	void EndPerfectParryFeedback();

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryCooldown = 4.f;

	bool bIsParryOnCooldown = false;

	FTimerHandle ParryCooldownTimer;

	// 服务于冷却
	void StartParryCooldown();
	void EndParryCooldown();

	UFUNCTION(BlueprintPure, Category = "Parry")
	bool IsParryOnCooldown() const
	{
		return bIsParryOnCooldown;
	}

	UFUNCTION(BlueprintPure, Category = "Parry")
	float GetParryCooldownPercent() const;

	UPROPERTY(EditAnywhere, Category = "Parry")
	UAnimMontage* ParryMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Skill State")
	UMaterialInterface* StateGlowMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StateGlowMID = nullptr;

public:

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* AttackMontage = nullptr;

	UFUNCTION(BlueprintPure, Category = "Energy")
	int32 GetCurrentEnergy() const
	{
		return CurrentEnergy;
	}

	UFUNCTION(BlueprintPure, Category = "Energy")
	int32 GetMaxEnergy() const
	{
		return MaxEnergy;
	}

	void RegenEnergy();  // 自然恢复能量

	void AddEnergy(int32 Amount);  // 格挡成功恢复能量

	bool ConsumeEnergy(int32 cost);  // 花费能量升级技能

	void ParryCooldownCleared();

	UPROPERTY(EditAnywhere, Category = "Skill State")
	UInputAction* UpgradeAction;

public:

	UPROPERTY(BlueprintAssignable, Category = "Energy")
	FOnEnergyChanged OnEnergyChanged;

	ESkillPowerState SkillPowerState = ESkillPowerState::Base;

	void TryUpgradeSkillState();  // 尝试升级
	void SetSkillPowerState(ESkillPowerState NewState);    // 直接改变强化状态
};

