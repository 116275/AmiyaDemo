// Fill out your copyright notice in the Description page of Project Settings.


#include "AmiyaDemoGameState.h"
#include "Net/UnrealNetwork.h"
#include "AmiyaDemoGameMode.h"

void AAmiyaDemoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAmiyaDemoGameState, CoinCount);
    DOREPLIFETIME(AAmiyaDemoGameState, CoinGoal);
    DOREPLIFETIME(AAmiyaDemoGameState, KillCount);
    DOREPLIFETIME(AAmiyaDemoGameState, KillGoal);
    
    DOREPLIFETIME(AAmiyaDemoGameState,CurrentBoss);

    DOREPLIFETIME(AAmiyaDemoGameState, GameResult);
}

void AAmiyaDemoGameState::SetCoinCount(int32 NewValue)
{
    if (!HasAuthority())
    {
        return;
    }

    CoinCount = NewValue;
}

void AAmiyaDemoGameState::SetKillCount(int32 NewValue)
{
    if (!HasAuthority())
    {
        return;
    }

    KillCount = NewValue;
}

void AAmiyaDemoGameState::SetGoals(int32 NewCoinGoal, int32 NewKillGoal)
{
    if (!HasAuthority())
    {
        return;
    }

    CoinGoal = NewCoinGoal;
    KillGoal = NewKillGoal;
}

void AAmiyaDemoGameState::SetCurrentBoss(ABossCharacter* NewBoss)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentBoss = NewBoss;

    OnBossSpawned.Broadcast(CurrentBoss);
}

void AAmiyaDemoGameState::OnRep_CurrentBoss()
{
    OnBossSpawned.Broadcast(CurrentBoss);
}

void AAmiyaDemoGameState::SetGameResult(EGameResult NewResult)
{
    if (!HasAuthority()) return;

    if (GameResult == NewResult) return;

    GameResult = NewResult;

    OnRep_GameResult();
}

void AAmiyaDemoGameState::OnRep_GameResult()
{
    OnResultChanged.Broadcast(GameResult);
}
