// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ACoinActor.generated.h"

class USphereComponent;
class UstaticMeshComponent;
class AmiyaDemoCharacter;

UCLASS()
class AMIYADEMO_API AACoinActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AACoinActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(VisibleAnywhere, Category = "Coin")
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere, Category = "Coin")
	UStaticMeshComponent* Mesh;

	UFUNCTION()
	void Overlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);  // ¼ì²âÅö×²

};
