// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "Net/UnrealNetwork.h"

void ALobbyPlayerController::ServerSelectCharacter_Implementation(ELobbyCharacter NewCharacter)
{
    ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>();

    if (!LobbyPS)
    {
        return;
    }

    LobbyPS->SetSelectedCharacter(NewCharacter);
}

