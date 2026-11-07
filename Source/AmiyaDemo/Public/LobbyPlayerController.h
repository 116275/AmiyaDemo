// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMIYADEMO_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerSelectCharacter(ELobbyCharacter NewCharacter);

};
