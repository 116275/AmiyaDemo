// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ALobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyChanged);

/**
 * 
 */
UCLASS()
class AMIYADEMO_API AALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY(ReplicatedUsing = OnRep_LobbyRevision)
	int32 LobbyRevision = 0;
	
	UFUNCTION()
	void OnRep_LobbyRevision();

	void MarkLobbyChanged();

	UPROPERTY(BlueprintAssignable)
	FOnLobbyChanged OnLobbyChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
