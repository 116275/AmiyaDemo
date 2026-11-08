// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmiyaDemoGameMode.h"
#include "AmiyaMultiplayerGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMIYADEMO_API AAmiyaMultiplayerGameMode : public AAmiyaDemoGameMode
{
	GENERATED_BODY()
	
public:

	AAmiyaMultiplayerGameMode();

	virtual void BeginPlay() override;

protected:
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    UPROPERTY(EditDefaultsOnly, Category = "Multiplayer|Character")
    TSubclassOf<APawn> AmiyaPawnClass;

    UPROPERTY(EditDefaultsOnly, Category = "Multiplayer|Character")
    TSubclassOf<APawn> CharacterBPawnClass;
};
