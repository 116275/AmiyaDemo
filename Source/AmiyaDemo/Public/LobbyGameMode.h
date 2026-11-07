// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMIYADEMO_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	bool CanStartGame() const;

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TSoftObjectPtr<UWorld> GameplayMap;

};
