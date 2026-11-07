// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ALobbyGameState.h"

void ALobbyPlayerState::SetSelectedCharacter_Implementation(ELobbyCharacter NewCharacter)  // 服务器才可以真正设置角色
{
    if (!HasAuthority())
    {
        return;
    }

    SelectedCharacter = NewCharacter;

    OnRep_SelectedCharacter();   // Server 自己不会自动执行 OnRep
}

void ALobbyPlayerState::OnRep_SelectedCharacter()
{
    if (AALobbyGameState* GS = GetWorld()->GetGameState<AALobbyGameState>())
    {
        GS->OnLobbyChanged.Broadcast();
    }
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALobbyPlayerState, SelectedCharacter);
    DOREPLIFETIME(ALobbyPlayerState, bIsHost);
}

void ALobbyPlayerState::SetIsHost(bool bNewIsHost)
{
    if (!HasAuthority())
    {
        return;
    }

    bIsHost = bNewIsHost;
    OnRep_IsHost();
}

void ALobbyPlayerState::OnRep_IsHost()
{
    if (AALobbyGameState* GS = GetWorld()->GetGameState<AALobbyGameState>())
    {
        GS->OnLobbyChanged.Broadcast();
    }
}