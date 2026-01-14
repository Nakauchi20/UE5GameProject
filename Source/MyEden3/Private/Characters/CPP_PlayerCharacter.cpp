// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CPP_PlayerCharacter.h"
#include "Framework/CPP_PlayerStateBase.h"
#include "GAS/CPP_PlayerAttributeSet.h"
#include "Characters/CPP_CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

ACPP_PlayerCharacter::ACPP_PlayerCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void ACPP_PlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeAbilitySystem();
}

void ACPP_PlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeAbilitySystem();
}

void ACPP_PlayerCharacter::InitializeAbilitySystem()
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    if (!PS || bAbilitySystemInitialized)
    {
        return;
    }

    PS->InitializeAbilitySystem(this);
    bAbilitySystemInitialized = true;
    OnAbilitySystemInitializedEvent();
}

// ============== IAbilitySystemInterface ==============
UAbilitySystemComponent* ACPP_PlayerCharacter::GetAbilitySystemComponent() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

// ============== Core Accessors ==============
ACPP_PlayerStateBase* ACPP_PlayerCharacter::GetPlayerStateBase() const
{
    return Cast<ACPP_PlayerStateBase>(GetPlayerState());
}

UCPP_PlayerAttributeSet* ACPP_PlayerCharacter::GetPlayerAttributeSet() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->PlayerAttributeSet : nullptr;
}

// ============== Sliding System Implementation ==============
void ACPP_PlayerCharacter::OnSlidingStarted()
{
    // C++側での処理（必要に応じて）
    // Blueprintイベントを呼び出し
    OnSlidingStartedEvent();

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: Sliding Started"));
#endif
}

void ACPP_PlayerCharacter::OnSlidingEnded()
{
    // C++側での処理（必要に応じて）
    // Blueprintイベントを呼び出し
    OnSlidingEndedEvent();

    // 注意: GA_Slideがポーリングで終了検知するため、
    // ここでGameplayEventを送信する必要はなくなりました

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: Sliding Ended"));
#endif
}

bool ACPP_PlayerCharacter::IsSliding() const
{
    if (const UCPP_CharacterMovementComponent* CustomMovement = GetCustomMovementComponent())
    {
        return CustomMovement->IsSliding();
    }
    return false;
}

void ACPP_PlayerCharacter::ForceStandUp()
{
    UCPP_CharacterMovementComponent* CustomMovement = GetCustomMovementComponent();
    if (!CustomMovement)
    {
        return;
    }

    // スライディング中の場合は停止
    if (CustomMovement->IsSliding())
    {
        CustomMovement->StopSliding();
    }

    // しゃがみ状態の場合は解除
    if (CustomMovement->IsCrouching())
    {
        CustomMovement->RequestUnCrouch();
    }
}