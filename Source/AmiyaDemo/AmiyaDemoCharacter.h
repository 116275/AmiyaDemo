// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Camera/CameraComponent.h"
#include "EnemyCharacter.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChainLightningAimChanged, bool, bIsAiming);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShuttleChargesChanged, int32, NewCharges);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUltimateStateChanged, bool, bIsActive);

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

	UPROPERTY(EditAnywhere, Category = "Input")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float SprintSpeed = 800.f;

	void StartSprint();

	void StopSprint();

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChainLightningAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CancelAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UltimateAction;

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

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformPunchHit();

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

	float CurrentHealth = 10000.f;
	float MaxHealth = 10000.f;

	float StartOffset = 50.f;
	float AttackRange = 100.f;
	float Radius = 50.f;
	float Damage = 50.f;

	float RegenTime = 2.f;   // 多少秒后开始回血
	float RegenAmountPercent = 2.f;   // 每秒回血百分比
	float TimeSinceLastDamage = 0.f;  // 离上一次受伤隔了多久

	UPROPERTY(EditAnywhere, Category = "Energy")
	int32 MaxEnergy = 8;

	int32 CurrentEnergy = 0;
	int32 EnergyRegen = 1;

	UPROPERTY(EditAnywhere, Category = "Energy")
	float EnergyRegenInterval = 0.5f;

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

private:

	void RegenEnergy();  // 自然恢复能量

	void AddEnergy(int32 Amount);  // 格挡成功恢复能量

	bool ConsumeEnergy(int32 cost);  // 花费能量升级技能

	void ParryCooldownCleared();

	UPROPERTY(EditAnywhere, Category = "Skill State")
	UInputAction* UpgradeAction;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 0.4f;

	bool bIsAttackOnCooldown = false;

	FTimerHandle AttackCooldownTimer;

	void StartAttackCooldown();
	void EndAttackCooldown();

	bool bIsChainLightningAiming = false;

	void HandleChainLightningInput();
	void StartChainLightningAim();
	void TryReleaseChainLightning();
	void CancelChainLightningAim();

	void CancelCurrentAction();

	AEnemyCharacter* CurrentChainLightningTarget = nullptr;

	void UpdateChainLightningTarget();

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ChainLightningTargetRadius = 175.f;

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ChainLightningAimFOV = 65.f;

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ChainLightningAimFade = 0.4f;

	void SetChainLightningAimFade(float Fade);

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	int32 ChainLightningEnergyCost = 3;

	void CastChainLightning(AEnemyCharacter* Target, ESkillPowerState CastState);

	// 优先攻击未被跳跃的敌人，最多跳三次
	TSet<AEnemyCharacter*> VisitedTargets;

	AEnemyCharacter* FindNextChainLightningTarget(AEnemyCharacter* FromTarget, const TSet<AEnemyCharacter*>& VisitedTargets);

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ChainLightningRadius = 600.f;

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ChainLightningMaxCastDistance = 900.f;   // 链式闪电最大施法距离

	bool bIsDead = false;

	UPROPERTY(EditAnywhere, Category = "Death")
	float DeathDestroyDelay = 3.f;

	void HandleDeath();

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ThunderstormRadius = 400.f;

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ThunderstormDamage = 25.f;

	void SpawnThunderstormAt(const FVector& Center, float NowRadius = 400.f, float NowDamage = 25.f);

	bool bIsSprinting = false;  // 是否在冲刺状态

	void RefreshMoveSpeed();

	bool bIsUltimateActive = false;   // 是否在放大招状态

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	int32 UltimateEnergyCost = 8;

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float UltimateDuration = 12.f;

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float UltimateMoveSpeedBonus = 800.f;

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float UltimateJumpBonus = 300.f;

	FTimerHandle UltimateTimer;

	void StartUltimate();
	void EndUltimate();

	float DefaultMaxAcceleration;
	float DefaultBrakingDeceleration;
	FRotator DefaultRotationRate;

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float UltimateMaxAcceleration = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float UltimateBrakingDeceleration = 4000.f;

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float UltimateRotationRate = 900.f;

	void HandleParryInput();
	void TryShuttle();

	bool PerformShuttle();

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	float ShuttleDistance = 600.f;

	UPROPERTY(EditAnywhere, Category = "Ultimate|Shuttle")
	float ShuttleDamage = 50.f;

	UPROPERTY(EditAnywhere, Category = "Ultimate|Shuttle")
	float ShuttleHitRadius = 80.f;

	void ApplyShuttlePathDamage(const FVector& Start, const FVector& End);

	bool bIsShuttling = false;

	UPROPERTY(EditAnywhere, Category = "Ultimate|Shuttle")
	float ShuttleDuration = 0.25f;   // 穿梭期间无敌

	FTimerHandle ShuttleTimer;

	void StartShuttle();   // 获得无敌
	void EndShuttle();  // 无敌结束

	int32 GetChainLightningEnergyCost() const;

public:

	UPROPERTY(BlueprintAssignable, Category = "Energy")
	FOnEnergyChanged OnEnergyChanged;

	ESkillPowerState SkillPowerState = ESkillPowerState::Base;

	void TryUpgradeSkillState();  // 尝试升级
	void SetSkillPowerState(ESkillPowerState NewState);    // 直接改变强化状态

	UPROPERTY(BlueprintAssignable, Category = "Chain Lightning")
	FOnChainLightningAimChanged OnChainLightningAimChanged;  // 瞄准状态改变委托

	UPROPERTY(EditAnywhere, Category = "Chain Lightning")
	float ChainLightningDamage = 100.f;

	void RewardEnergyForKill(int32 Amount)
	{
		AddEnergy(Amount);
	}

	UPROPERTY(EditAnywhere, Category = "Ultimate")
	int32 MaxShuttleCharges = 70;

	int32 RemainingShuttleCharges = 0;

	UPROPERTY(BlueprintAssignable, Category = "Ultimate")
	FOnShuttleChargesChanged OnShuttleChargesChanged;

	UFUNCTION(BlueprintPure, Category = "Ultimate")
	int32 GetRemainingShuttleCharges() const
	{
		return RemainingShuttleCharges;
	}

	UFUNCTION(BlueprintPure, Category = "Ultimate")
	float GetUltimateRemainingTime() const;

	UPROPERTY(BlueprintAssignable, Category = "Ultimate")
	FOnUltimateStateChanged OnUltimateStateChanged;

};

