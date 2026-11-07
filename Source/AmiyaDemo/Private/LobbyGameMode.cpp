// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "ALobbyGameState.h"

ALobbyGameMode::ALobbyGameMode()
{
    PlayerStateClass = ALobbyPlayerState::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (ALobbyPlayerState* PS = NewPlayer->GetPlayerState<ALobbyPlayerState>())
    {
        PS->SetIsHost(NewPlayer->IsLocalController());
    }

    if (AALobbyGameState* GS = GetGameState<AALobbyGameState>())
    {
        GS->MarkLobbyChanged();
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (AALobbyGameState* GS = GetGameState<AALobbyGameState>())
    {
        GS->MarkLobbyChanged();
    }
}

bool ALobbyGameMode::CanStartGame() const  // 当游戏人数满足 2 人且都选了角色，就可以开始游戏
{
    const AGameStateBase* GS = GameState;

    if (!GS || GS->PlayerArray.Num() != 2)
    {
        return false;
    }

    for (APlayerState* PlayerState : GS->PlayerArray)
    {
        const ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PlayerState);

        if (!LobbyPS || LobbyPS->GetSelectedCharacter() == ELobbyCharacter::None)
        {
            return false;
        }
    }

    return true;
}

void ALobbyGameMode::StartGame()
{
    if (!HasAuthority() || !CanStartGame())
    {
        return;
    }

    if (GameplayMap.IsNull())
    {
        return;
    }

    const FString MapPath = GameplayMap.ToSoftObjectPath().GetLongPackageName();

    GetWorld()->ServerTravel(MapPath);
}


