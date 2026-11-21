// Fill out your copyright notice in the Description page of Project Settings.


#include "TalulahCharacter.h"
#include "AbilitySystemComponent.h"
#include "TalulahAttributeSet.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "GameFrameWork/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"

ATalulahCharacter::ATalulahCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	ASC =
		CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	ASC->SetIsReplicated(true);

	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AS =
		CreateDefaultSubobject<UTalulahAttributeSet>(TEXT("AttributeSet"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	FollowCamera->SetActive(true);

	bUseControllerRotationPitch = false;
	//bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

}

void ATalulahCharacter::InputMove(const FInputActionValue& Value)
{
	const FVector2D Movement = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector Forward =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector Right =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Movement.Y);
	AddMovementInput(Right, Movement.X);

}

void ATalulahCharacter::InputLook(const FInputActionValue& Value)
{
	const FVector2D Look = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	AddControllerYawInput(Look.X);
	AddControllerPitchInput(Look.Y);
}

// Called when the game starts or when spawned
void ATalulahCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called to bind functionality to input
void ATalulahCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(
				AttackAction,
				ETriggerEvent::Started,
				this,
				&ATalulahCharacter::InputAttack
			);
		}

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&ATalulahCharacter::InputMove
		);

		EnhancedInputComponent->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&ATalulahCharacter::InputLook
		);

		if (MouseLookAction)
		{
			EnhancedInputComponent->BindAction(
				MouseLookAction,
				ETriggerEvent::Triggered,
				this,
				&ATalulahCharacter::InputLook
			);
		}

		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&ACharacter::Jump
		);

		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&ACharacter::StopJumping
		);
	}
}

UAbilitySystemComponent* ATalulahCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void ATalulahCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

	if (HasAuthority() && ASC && TestAbilityClass)
	{
		ASC->GiveAbility(
			FGameplayAbilitySpec(TestAbilityClass, 1, 0)
		);
	}
}

void ATalulahCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	InitAbilityActorInfo();
}

void ATalulahCharacter::InitAbilityActorInfo()
{
	if (!ASC)
	{
		return;
	}

	ASC->InitAbilityActorInfo(
		this,
		this
	);
}

void ATalulahCharacter::InputAttack()
{
	if (!ASC || !AttackAction)
	{
		return;
	}

	ASC->TryActivateAbilityByClass(TestAbilityClass);
}

void ATalulahCharacter::PerformAttackTrace()  // 平 A 检测
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	FVector Forward = GetActorForwardVector();
	
	FVector Start = GetActorLocation() + Forward * 80.f;
	FVector End = Start + Forward * 180.f;
	float Radius = 80.f;

	FHitResult HitResult;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(TalulahAttackTrace),
		false
	);

	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	FVector TraceVector = End - Start;
	FVector Center = (Start + End) * 0.5f;

	FQuat Rotation =
		FRotationMatrix::MakeFromZ(TraceVector).ToQuat();

	float HalfHeight = TraceVector.Size() * 0.5f + Radius;

	DrawDebugCapsule(
		GetWorld(),
		Center,
		HalfHeight,
		Radius,
		Rotation,
		bHit ? FColor::Green : FColor::Red,
		false,
		1.f
	);

	if (bHit)
	{
		DrawDebugSphere(
			GetWorld(),
			HitResult.ImpactPoint,
			12.f,
			12,
			FColor::Yellow,
			false,
			1.f
		);

		AActor* Target = HitResult.GetActor();

		if (!IsValid(Target) || Target == this)
		{
			return;
		}

		IAbilitySystemInterface* TargetASI =
			Cast<IAbilitySystemInterface>(Target);

		if (!TargetASI)
		{
			return;
		}

		UAbilitySystemComponent* TargetASC =
			TargetASI->GetAbilitySystemComponent();

		if (!TargetASC || !ASC || !BasicAttackDamageEffectClass)
		{
			return;
		}

		// Damage
		FGameplayEffectContextHandle EffectContext =
			ASC->MakeEffectContext();

		EffectContext.AddSourceObject(this);

		ASC->ApplyGameplayEffectToTarget(
			BasicAttackDamageEffectClass->GetDefaultObject<UGameplayEffect>(),
			TargetASC,
			1.0f,
			EffectContext
		);
	}

}

void ATalulahCharacter::HandleDeath()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;

	MulticastHandleDeath();

	SetLifeSpan(5.f);
}

void ATalulahCharacter::MulticastHandleDeath_Implementation()
{
	bIsDead = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnDeathVisuals();   // 蓝图处理组件

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		CharacterMesh->SetSimulatePhysics(true);
		CharacterMesh->WakeAllRigidBodies();
	}
}

