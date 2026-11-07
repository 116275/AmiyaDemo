// Fill out your copyright notice in the Description page of Project Settings.


#include "ALobbyGameState.h"
#include "Net/UnrealNetwork.h"

void AALobbyGameState::MarkLobbyChanged()
{
    if (!HasAuthority())
    {
        return;
    }

    ++LobbyRevision;

    OnRep_LobbyRevision();
}

void AALobbyGameState::OnRep_LobbyRevision()
{
    OnLobbyChanged.Broadcast();
}

void AALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AALobbyGameState, LobbyRevision);
}