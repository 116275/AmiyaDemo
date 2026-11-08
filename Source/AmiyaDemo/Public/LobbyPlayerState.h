// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectedCharacterChanged);

UENUM(BlueprintType)
enum class ELobbyCharacter : uint8
{
    None,
    Amiya,
    CharacterB
};

UCLASS()
class AMIYADEMO_API ALobbyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:

    UPROPERTY(ReplicatedUsing = OnRep_SelectedCharacter)
    ELobbyCharacter SelectedCharacter = ELobbyCharacter::None;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void SetSelectedCharacter(ELobbyCharacter NewCharacter);

    UFUNCTION()
    void OnRep_SelectedCharacter();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure)
    ELobbyCharacter GetSelectedCharacter() const
    {
        return SelectedCharacter;
    }

    //UPROPERTY(BlueprintAssignable, Category = "Lobby")
    //FOnSelectedCharacterChanged OnSelectedCharacterChanged;

    UPROPERTY(ReplicatedUsing = OnRep_IsHost)
    bool bIsHost = false;

    void SetIsHost(bool bNewIsHost);

    UFUNCTION()
    void OnRep_IsHost();

    UFUNCTION(BlueprintPure)
    bool IsHost() const
    {
        return bIsHost;
    }

protected:

    virtual void CopyProperties(APlayerState* PlayerState) override;

};
