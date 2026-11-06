// Fill out your copyright notice in the Description page of Project Settings.


#include "AmiyaMultiplayerGameMode.h"

bool AAmiyaMultiplayerGameMode::AllowPausing(APlayerController* PC)
{
    if (!PC)
    {
        return false;
    }

    return PC->IsLocalController();
}