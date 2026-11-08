// Fill out your copyright notice in the Description page of Project Settings.


#include "AmiyaMultiplayerGameMode.h"
#include "GameFramework/GameState.h"
#include "LobbyPlayerState.h"

AAmiyaMultiplayerGameMode::AAmiyaMultiplayerGameMode()
{
    PlayerStateClass = ALobbyPlayerState::StaticClass();
}

void AAmiyaMultiplayerGameMode::BeginPlay()
{
    Super::BeginPlay();
}

UClass* AAmiyaMultiplayerGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if (!InController)
    {
        return Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    ALobbyPlayerState* LobbyPS = InController->GetPlayerState<ALobbyPlayerState>();

    if (!LobbyPS)
    {
        return Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    switch (LobbyPS->GetSelectedCharacter())
    {
    case ELobbyCharacter::Amiya:
        if (AmiyaPawnClass)
        {
            return AmiyaPawnClass;
        }
        break;

    case ELobbyCharacter::CharacterB:
        if (CharacterBPawnClass)
        {
            return CharacterBPawnClass;
        }
        break;

    case ELobbyCharacter::None:
    default:
        break;
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}