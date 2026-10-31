// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AmiyaDemoGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class AMIYADEMO_API UAmiyaDemoGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "Character")
	TSubclassOf<APawn> SelectedCharacterClass;

};
