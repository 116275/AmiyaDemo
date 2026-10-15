// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmiyaDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AmiyaDemo.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EnemyCharacter.h"
#include "ThunderstormDamageType.h"
#include "Engine/OverlapResult.h"

AAmiyaDemoCharacter::AAmiyaDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 1.f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AAmiyaDemoCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultMaxAcceleration = GetCharacterMovement()->MaxAcceleration;

	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;

	DefaultRotationRate = GetCharacterMovement()->RotationRate;

	if (StateGlowMaterial)
	{
		StateGlowMID = UMaterialInstanceDynamic::Create(StateGlowMaterial, this);
	}

	SkillPowerState = ESkillPowerState::Base;

	if (FollowCamera)
	{
		DefaultFOV = FollowCamera->FieldOfView;
	}

	GetWorld()->GetTimerManager().SetTimer(EnergyRegenTimer, this, &AAmiyaDemoCharacter::RegenEnergy, EnergyRegenInterval, true);
}

void AAmiyaDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAmiyaDemoCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAmiyaDemoCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAmiyaDemoCharacter::Look);

		// Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::Attack);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AAmiyaDemoCharacter::StopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AAmiyaDemoCharacter::StopSprint);
		// 防止意外退出

		// Parry
		EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::HandleParryInput);

		// UPgrade
		EnhancedInputComponent->BindAction(UpgradeAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::TryUpgradeSkillState);

		// ChainLightning
		EnhancedInputComponent->BindAction(ChainLightningAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::HandleChainLightningInput);

		// Cancel
		EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::CancelCurrentAction);
	
		// Ultimate
		EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &AAmiyaDemoCharacter::StartUltimate);
	}
	else
	{
		UE_LOG(LogAmiyaDemo, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAmiyaDemoCharacter::Tick(float Delta)
{
	Super::Tick(Delta);
	TimeSinceLastDamage += Delta;

	if (CurrentHealth < MaxHealth && TimeSinceLastDamage >= RegenTime)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth + RegenAmountPercent / 100 * MaxHealth * Delta, 0.f, MaxHealth);
	}

	if (bIsChainLightningAiming)
	{
		UpdateChainLightningTarget();
	}
}

void AAmiyaDemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAmiyaDemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AAmiyaDemoCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{

		if (bIsParrying && (!FMath::IsNearlyZero(Right) || !FMath::IsNearlyZero(Forward)))
		{
			EndParryState();
		}

		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAmiyaDemoCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAmiyaDemoCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AAmiyaDemoCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AAmiyaDemoCharacter::Attack()
{
	if (bIsAttackOnCooldown)
	{
		return;
	}

	StartAttackCooldown();

	CancelCurrentAction();


	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void AAmiyaDemoCharacter::PerformPunchHit()
{
	FVector ActorLocal = AAmiyaDemoCharacter::GetActorLocation();
	FVector Forward = AAmiyaDemoCharacter::GetActorForwardVector();
	FVector Start = ActorLocal + Forward * StartOffset;
	FVector End = Start + Forward * AttackRange;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	FCollisionShape Sphere;

	Sphere = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActor(this);
	bool BHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, Sphere, Params);

	if (BHit)
	{
		if (HitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitResult.GetActor(), Damage, GetController(), this, UDamageType::StaticClass());
		}
	}

}

float AAmiyaDemoCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || bIsShuttling)
	{
		return 0.f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage <= 0.f)
	{
		return 0.f;
	}

	if (bIsParrying)
	{
		if (bIsPerfectParrying)
		{
			AddEnergy(2);

			EndParry();
			ParryCooldownCleared();

			StartPerfectParryFeedback();

			return 0.f;
		}
		
		ActualDamage *= 0.5;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.f, MaxHealth);

	TimeSinceLastDamage = 0;

	if (CurrentHealth <= 0)
	{
		HandleDeath();
	}

	return ActualDamage;
}

void AAmiyaDemoCharacter::StartSprint()
{
	bIsSprinting = true;
	RefreshMoveSpeed();
}

void AAmiyaDemoCharacter::StopSprint()
{
	bIsSprinting = false;
	RefreshMoveSpeed();
}

void AAmiyaDemoCharacter::RefreshMoveSpeed()
{
	float NewSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;

	if (bIsUltimateActive)
	{
		NewSpeed += UltimateMoveSpeedBonus;
	}

	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AAmiyaDemoCharacter::RegenEnergy()
{
	AddEnergy(EnergyRegen);
}

void AAmiyaDemoCharacter::AddEnergy(int32 Amount)
{
	CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0, MaxEnergy);

	OnEnergyChanged.Broadcast(CurrentEnergy);
}

bool AAmiyaDemoCharacter::ConsumeEnergy(int32 cost)
{
	if (cost > 0 && CurrentEnergy >= cost)
	{
		CurrentEnergy -= cost;

		OnEnergyChanged.Broadcast(CurrentEnergy);

		return true;
	}
	else
	{
		return false;
	}
}

void AAmiyaDemoCharacter::StartParryState()
{
	if (bIsParrying || bIsParryOnCooldown)
	{
		return;
	}

	CancelCurrentAction();

	if (ParryMontage)
	{
		PlayAnimMontage(ParryMontage);
	}

	bIsParrying = true;
	bIsPerfectParrying = true;

	GetWorld()->GetTimerManager().SetTimer(PerfectParryTimer, this, &AAmiyaDemoCharacter::EndPerfectParry, PerfectParryDuration, false);

	GetWorld()->GetTimerManager().SetTimer(ParryTimer, this, &AAmiyaDemoCharacter::EndParryState, ParryDuration, false);

}

void AAmiyaDemoCharacter::EndParry()  // 负责防反收尾
{
	bIsParrying = false;
	bIsPerfectParrying = false;

	GetWorld()->GetTimerManager().ClearTimer(PerfectParryTimer);
	GetWorld()->GetTimerManager().ClearTimer(ParryTimer);

	if (ParryMontage)
	{
		StopAnimMontage(ParryMontage);
	}
}

void AAmiyaDemoCharacter::EndPerfectParry()  // 负责防反收尾
{
	bIsPerfectParrying = false;

	GetWorld()->GetTimerManager().ClearTimer(PerfectParryTimer);
}

void AAmiyaDemoCharacter::StartPerfectParryFeedback()
{
	FollowCamera->SetFieldOfView(PerfectParryFOV);

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), PerfectParrySlowFactor);

	GetWorld()->GetTimerManager().SetTimer(PerfectParryFeedbackTimer, this, &AAmiyaDemoCharacter::EndPerfectParryFeedback, HighLightDuration * PerfectParrySlowFactor, false);
}

void AAmiyaDemoCharacter::EndPerfectParryFeedback()
{
	FollowCamera->SetFieldOfView(DefaultFOV);

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
}

void AAmiyaDemoCharacter::EndParryState()  // 总的负责收尾(要进入冷却)
{
	if (!bIsParrying)
	{
		return;
	}

	EndParry();
	StartParryCooldown();
}

void AAmiyaDemoCharacter::StartParryCooldown()
{
	bIsParryOnCooldown = true;

	GetWorld()->GetTimerManager().SetTimer(ParryCooldownTimer, this, &AAmiyaDemoCharacter::EndParryCooldown, ParryCooldown, false);
}

void AAmiyaDemoCharacter::EndParryCooldown()
{
	bIsParryOnCooldown = false;
}

void AAmiyaDemoCharacter::ParryCooldownCleared()
{
	GetWorld()->GetTimerManager().ClearTimer(ParryCooldownTimer);
	bIsParryOnCooldown = false;
}

float AAmiyaDemoCharacter::GetParryCooldownPercent() const
{
	if (!GetWorld() || !bIsParryOnCooldown)
	{
		return 0.f;
	}

	const float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ParryCooldownTimer);

	return FMath::Clamp(RemainingTime / ParryCooldown, 0.f, 1.f);
}

void AAmiyaDemoCharacter::TryUpgradeSkillState()
{
	switch (SkillPowerState)
	{
	case ESkillPowerState::Base :
		if (ConsumeEnergy(3))
		{
			SetSkillPowerState(ESkillPowerState::Enhanced);
		}
		break;

	case ESkillPowerState::Enhanced :
		if (ConsumeEnergy(2))
		{
			SetSkillPowerState(ESkillPowerState::Overcharged);
		}
		break;

	case ESkillPowerState::Overcharged :
		break;
	}
}

void AAmiyaDemoCharacter::SetSkillPowerState(ESkillPowerState NewState)
{
	SkillPowerState = NewState;

	switch (SkillPowerState)
	{
	case ESkillPowerState::Base :
		GetMesh()->SetOverlayMaterial(nullptr);
		break;

	case ESkillPowerState::Enhanced :
		if (StateGlowMID)
		{
			StateGlowMID->SetVectorParameterValue(TEXT("GlowColor"), FLinearColor::Yellow);

			GetMesh()->SetOverlayMaterial(StateGlowMID);
		}
		break;

	case ESkillPowerState::Overcharged :
		if (StateGlowMID)
		{
			StateGlowMID->SetVectorParameterValue(TEXT("GlowColor"), FLinearColor::Red);

			GetMesh()->SetOverlayMaterial(StateGlowMID);
		}
		break;
	}
}

void AAmiyaDemoCharacter::StartAttackCooldown()
{
	bIsAttackOnCooldown = true;

	GetWorld()->GetTimerManager().SetTimer(AttackCooldownTimer, this, &AAmiyaDemoCharacter::EndAttackCooldown, AttackCooldown, false);
}

void AAmiyaDemoCharacter::EndAttackCooldown()
{
	bIsAttackOnCooldown = false;
}

void AAmiyaDemoCharacter::CancelCurrentAction()   // 通用取消当前动作（用于技能）
{
	if (bIsChainLightningAiming)
	{
		CancelChainLightningAim();
	}

	if (bIsParrying)
	{
		EndParryState();
	}
}

void AAmiyaDemoCharacter::HandleChainLightningInput()
{
	if (!bIsChainLightningAiming)
	{
		StartChainLightningAim();
		return;
	}

	TryReleaseChainLightning();
}

void AAmiyaDemoCharacter::StartChainLightningAim()
{
	if (bIsChainLightningAiming)
	{
		return;
	}

	if (CurrentEnergy < GetChainLightningEnergyCost())
	{
		return;
	}

	// 后面再补：
	// if (bChainLightningOnCooldown) return;

	CancelCurrentAction();

	bIsChainLightningAiming = true;
	CurrentChainLightningTarget = nullptr;

	if (FollowCamera)
	{
		FollowCamera->SetFieldOfView(ChainLightningAimFOV);
	}

	SetChainLightningAimFade(ChainLightningAimFade);

	OnChainLightningAimChanged.Broadcast(true);

	// 后续：
	// ShowCrosshair();
	// ZoomCamera();
}

void AAmiyaDemoCharacter::TryReleaseChainLightning()
{
	if (!bIsChainLightningAiming)   // 如果没在瞄准
	{
		return;
	}

	if (!IsValid(CurrentChainLightningTarget))   // 如果没有目标，取消释放(或目标死亡)
	{
		CancelChainLightningAim();
		return;
	}

	if (!ConsumeEnergy(GetChainLightningEnergyCost()))  // 如果能量不足
	{
		CancelChainLightningAim();
		return;
	}

	// 真正确认释放以后，才开始产生代价
	bIsChainLightningAiming = false;

	if (FollowCamera)
	{
		FollowCamera->SetFieldOfView(DefaultFOV);
	}

	SetChainLightningAimFade(1.0f);

	if (IsValid(CurrentChainLightningTarget))
	{
		CurrentChainLightningTarget->SetChainLightningTargetMaterial(false);
	}

	AEnemyCharacter* Target = CurrentChainLightningTarget;

	CurrentChainLightningTarget = nullptr;   // 先保存再清瞄准状态

	OnChainLightningAimChanged.Broadcast(false);

	const ESkillPowerState CastState = SkillPowerState;  // 释放时的技能状态

	CastChainLightning(Target, CastState);

	SetSkillPowerState(ESkillPowerState::Base);

	// StartCooldown(...)
}

void AAmiyaDemoCharacter::CancelChainLightningAim()
{
	if (!bIsChainLightningAiming)
	{
		return;
	}

	bIsChainLightningAiming = false;

	if (FollowCamera)
	{
		FollowCamera->SetFieldOfView(DefaultFOV);
	}

	SetChainLightningAimFade(1.0f);
	SetSkillPowerState(SkillPowerState);

	if (IsValid(CurrentChainLightningTarget))
	{
		CurrentChainLightningTarget->SetChainLightningTargetMaterial(false);
	}

	CurrentChainLightningTarget = nullptr;

	OnChainLightningAimChanged.Broadcast(false);

	// 后续：
	// HideCrosshair();
	// RestoreCameraFOV();
	// RemoveTargetHighlight();
}

void AAmiyaDemoCharacter::UpdateChainLightningTarget()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		return;
	}

	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	const FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), Enemies);

	AEnemyCharacter* BestTarget = nullptr;
	float BestDistance = TNumericLimits<float>::Max();

	for (AActor* Actor : Enemies)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);

		if (!IsValid(Enemy) || !Enemy->IsTargetable())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());

		if (DistanceSq > FMath::Square(ChainLightningMaxCastDistance))   // 小优化
		{
			continue;
		}

		FVector2D ScreenPosition;

		const bool bProjected = PlayerController->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenPosition);

		if (!bProjected)
		{
			continue;
		}

		float ScreenDistance = FVector2D::Distance(ScreenPosition, ScreenCenter);

		if (ScreenDistance <= ChainLightningTargetRadius && ScreenDistance < BestDistance)
		{
			BestDistance = ScreenDistance;
			BestTarget = Enemy;
		}
	}

	if (BestTarget != CurrentChainLightningTarget)
	{
		if (IsValid(CurrentChainLightningTarget))
		{
			CurrentChainLightningTarget->SetChainLightningTargetMaterial(false);
		}

		CurrentChainLightningTarget = BestTarget;

		if (IsValid(CurrentChainLightningTarget))
		{
			CurrentChainLightningTarget->SetChainLightningTargetMaterial(true);
		}
	}
}

void AAmiyaDemoCharacter::SetChainLightningAimFade(float Fade)
{
	GetMesh()->SetScalarParameterValueOnMaterials(TEXT("AimFade"), Fade);

	GetMesh()->SetOverlayMaterial(nullptr);
}

void AAmiyaDemoCharacter::CastChainLightning(AEnemyCharacter* Target, ESkillPowerState CastState)
{
	if (!IsValid(Target))
	{
		return;
	}

	int32 MaxTargets = 3;
	float DamageDecay = 0.75f;

	switch (CastState)
	{
	case ESkillPowerState::Base :
		MaxTargets = 3;
		DamageDecay = 0.75f;
		break;
	case ESkillPowerState::Enhanced :
		MaxTargets = 4;
		DamageDecay = 1.f;
		break;
	case ESkillPowerState::Overcharged :
		MaxTargets = 6;
		DamageDecay = 1.f;
		break;
	}

	TSet<AEnemyCharacter*> HitTargets;
	AEnemyCharacter* CurrentTarget = Target;  // 链式闪电当前跳到的敌人
	float CurrentDamage = ChainLightningDamage;  // 当前链式闪电伤害

	FVector ChainStart = GetActorLocation() + FVector(0.f, 0.f, 75.f);

	for (int32 HitIndex = 0; HitIndex < MaxTargets; HitIndex++)
	{
		if (!IsValid(CurrentTarget))
		{
			break;
		}

		const FVector TargetLocation = CurrentTarget->GetActorLocation() + FVector(0.f, 0.f, 75.f);

		DrawDebugLine(GetWorld(), ChainStart, TargetLocation, FColor::Yellow, false, 0.75f, 0, 5.f);

		HitTargets.Add(CurrentTarget);

		AEnemyCharacter* NextTarget = nullptr;

		if (HitIndex + 1 < MaxTargets)
		{
			NextTarget = FindNextChainLightningTarget(CurrentTarget, HitTargets);
		}

		UGameplayStatics::ApplyDamage(CurrentTarget, CurrentDamage, GetController(), this, UDamageType::StaticClass());

		ChainStart = TargetLocation;
		CurrentTarget = NextTarget;
		CurrentDamage *= DamageDecay;
	}

	if (CastState == ESkillPowerState::Overcharged)  // 状态2时，技能结束释放雷暴
	{
		for (AEnemyCharacter* HitTarget : HitTargets)
		{
			if (!IsValid(HitTarget))
			{
				continue;
			}

			const FVector StormCenter =
				HitTarget->GetActorLocation();

			SpawnThunderstormAt(HitTarget->GetBodyLocation());
		}
	}

}

void AAmiyaDemoCharacter::SpawnThunderstormAt(const FVector& StormCenter, float NowRadius, float NowDamage)
{
	DrawDebugSphere(GetWorld(), StormCenter, NowRadius, 24, FColor::Purple, false, 1.5f, 0, 3.f);

	TArray<AActor*> Enemies;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), Enemies);

	const float Pow = FMath::Square(NowRadius);

	for (AActor* Actor : Enemies)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);

		if (!IsValid(Enemy) || !Enemy->IsTargetable())
		{
			continue;
		}

		float DistanceSq = FVector::DistSquared(StormCenter, Enemy->GetBodyLocation());

		if (DistanceSq > Pow)
		{
			continue;
		}

		UGameplayStatics::ApplyDamage(Enemy, NowDamage, GetController(), this, UThunderstormDamageType::StaticClass());
	}
}

AEnemyCharacter* AAmiyaDemoCharacter::FindNextChainLightningTarget(AEnemyCharacter* FromTarget, const TSet<AEnemyCharacter*>& HitTargets)
{
	if (!IsValid(FromTarget))
	{
		return nullptr;
	}

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), Enemies);

	AEnemyCharacter* BestUnvisitedTarget = nullptr;
	float BestUnvisitedDistance = TNumericLimits<float>::Max();

	AEnemyCharacter* BestFallbackTarget = nullptr;
	float BestFallbackDistance = TNumericLimits<float>::Max();

	const FVector FromLocation = FromTarget->GetActorLocation();

	for (AActor* Actor : Enemies)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);

		if (!IsValid(Enemy) || Enemy == FromTarget || !Enemy->IsTargetable())
		{
			continue;
		}

		const float Distance = FVector::Dist(FromLocation, Enemy->GetActorLocation());

		if (Distance > ChainLightningRadius)
		{
			continue;
		}

		if (Distance < BestFallbackDistance)
		{
			BestFallbackDistance = Distance;
			BestFallbackTarget = Enemy;
		}

		if (!HitTargets.Contains(Enemy) && Distance < BestUnvisitedDistance)
		{
			BestUnvisitedDistance = Distance;
			BestUnvisitedTarget = Enemy;
		}
	}

	return BestUnvisitedTarget ? BestUnvisitedTarget : BestFallbackTarget;
}

void AAmiyaDemoCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	CancelCurrentAction();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionEnabled(
		ECollisionEnabled::QueryAndPhysics
	);

	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();

	SetLifeSpan(DeathDestroyDelay);
}

void AAmiyaDemoCharacter::StartUltimate()
{
	if (bIsUltimateActive)
	{
		return;
	}

	if (!ConsumeEnergy(UltimateEnergyCost))
	{
		return;
	}

	bIsUltimateActive = true;
	OnUltimateStateChanged.Broadcast(true);

	RemainingShuttleCharges = MaxShuttleCharges;
	OnShuttleChargesChanged.Broadcast(RemainingShuttleCharges);

	RefreshMoveSpeed();

	GetCharacterMovement()->JumpZVelocity += UltimateJumpBonus;

	GetCharacterMovement()->MaxAcceleration = UltimateMaxAcceleration;

	GetCharacterMovement()->BrakingDecelerationWalking = UltimateBrakingDeceleration;

	GetCharacterMovement()->RotationRate = FRotator(0.f, UltimateRotationRate, 0.f);

	GetWorld()->GetTimerManager().SetTimer(UltimateTimer, this, &AAmiyaDemoCharacter::EndUltimate, UltimateDuration, false);
}

void AAmiyaDemoCharacter::EndUltimate()
{
	if (!bIsUltimateActive)
	{
		return;
	}

	bIsUltimateActive = false;
	OnUltimateStateChanged.Broadcast(false);

	RemainingShuttleCharges = 0;
	OnShuttleChargesChanged.Broadcast(RemainingShuttleCharges);

	RefreshMoveSpeed();

	GetCharacterMovement()->JumpZVelocity -= UltimateJumpBonus;

	GetCharacterMovement()->MaxAcceleration = DefaultMaxAcceleration;

	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;

	GetCharacterMovement()->RotationRate = DefaultRotationRate;

	GetWorld()->GetTimerManager().ClearTimer(UltimateTimer);

	SpawnThunderstormAt(GetActorLocation(), 1200.f, 125.f);   // 大招结束释放强化后的大雷暴
}

float AAmiyaDemoCharacter::GetUltimateRemainingTime() const
{
	if (!bIsUltimateActive)
	{
		return 0.f;
	}

	return FMath::Max(0.f, GetWorld()->GetTimerManager().GetTimerRemaining(UltimateTimer));
}

void AAmiyaDemoCharacter::HandleParryInput()
{
	if (bIsUltimateActive)
	{
		TryShuttle();
		return;
	}

	StartParryState();
}

void AAmiyaDemoCharacter::TryShuttle()  // 防止卡墙扣次数
{
	if (!bIsUltimateActive)
	{
		return;
	}

	if (RemainingShuttleCharges <= 0)
	{
		return;
	}

	if (!PerformShuttle())  // 如果瞬移失败（卡墙），不扣次数
	{
		return;
	}

	StartShuttle();   // 执行无敌逻辑

	--RemainingShuttleCharges;
	OnShuttleChargesChanged.Broadcast(RemainingShuttleCharges);
}

bool AAmiyaDemoCharacter::PerformShuttle()   // 真正实现瞬移
{
	if (!Controller)
	{
		return false;
	}

	FVector Direction = Controller->GetControlRotation().Vector();

	Direction.Z = 0.f;
	Direction.Normalize();

	const FVector StartLocation = GetActorLocation();
	const FVector Delta = Direction * ShuttleDistance;

	FHitResult Hit;

	UCapsuleComponent* Capsule = GetCapsuleComponent();

	const ECollisionResponse PreviousPawnResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);

	// Shuttle 时允许穿过其他 Pawn
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	AddActorWorldOffset(Delta, true, &Hit, ETeleportType::None);

	// 恢复原来的碰撞设置
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, PreviousPawnResponse);

	const FVector EndLocation = GetActorLocation();

	if (EndLocation.Equals(StartLocation, 1.f))
	{
		return false;
	}

	ApplyShuttlePathDamage(StartLocation, EndLocation);

	return true;
}

void AAmiyaDemoCharacter::ApplyShuttlePathDamage(const FVector& Start, const FVector& End)
{
	const FVector Path = End - Start;
	const float Distance = Path.Size();

	if (Distance <= KINDA_SMALL_NUMBER)  // 防止后续计算中出现“除以零”的错误
	{
		return;
	}

	const FVector Direction = Path / Distance;
	const FVector Center = (Start + End) * 0.5f;   // 不会出现“贴脸漏伤”的情况

	const float HalfHeight = Distance * 0.5f + ShuttleHitRadius;

	const FQuat Rotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();  // 将胶囊体的 Z 轴对齐到冲刺的 Direction 方向

	TArray<FOverlapResult> Overlaps;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);  // 忽略自身

	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);   // 表示这次物理检测只关心 Pawn 类型的物体

	const FCollisionShape Shape = FCollisionShape::MakeCapsule(ShuttleHitRadius, HalfHeight);  // 生成一个胶囊体，半径为 ShuttleHitRadius，高度为 HalfHeight

	// 在指定的中心点 Center、指定的旋转角度 Rotation 下生成这个胶囊体，并把碰到的所有 Pawn 记录到 Overlaps 数组中
	GetWorld()->OverlapMultiByObjectType(Overlaps, Center, Rotation, ObjectParams, Shape, QueryParams);

	TSet<AEnemyCharacter*> DamagedEnemies;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Overlap.GetActor());

		if (!Enemy || !Enemy->IsTargetable() || DamagedEnemies.Contains(Enemy))
		{
			continue;
		}

		DamagedEnemies.Add(Enemy);

		UGameplayStatics::ApplyDamage(Enemy, ShuttleDamage, GetController(), this, UDamageType::StaticClass());
	}
}

void AAmiyaDemoCharacter::StartShuttle()
{
	bIsShuttling = true;

	GetWorld()->GetTimerManager().SetTimer(ShuttleTimer, this, &AAmiyaDemoCharacter::EndShuttle, ShuttleDuration, false);
}

void AAmiyaDemoCharacter::EndShuttle()
{
	bIsShuttling = false;
}

int32 AAmiyaDemoCharacter::GetChainLightningEnergyCost() const
{
	if (bIsUltimateActive)
	{
		return FMath::Max(0, ChainLightningEnergyCost - 2);
	}

	return ChainLightningEnergyCost;
}