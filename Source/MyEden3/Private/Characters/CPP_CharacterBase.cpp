// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CPP_CharacterBase.h"
#include "Framework/CPP_PlayerStateBase.h"
#include "GAS/CPP_PlayerAttributeSet.h"
#include "Characters/CPP_CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ACPP_CharacterBase::ACPP_CharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCPP_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = false;

    // ネットワーク設定
    SetReplicateMovement(true);
    bReplicates = true;

}

void ACPP_CharacterBase::BeginPlay()
{
    Super::BeginPlay();
}

void ACPP_CharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeAbilitySystem();
}

void ACPP_CharacterBase::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeAbilitySystem();
}

void ACPP_CharacterBase::InitializeAbilitySystem()
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
UAbilitySystemComponent* ACPP_CharacterBase::GetAbilitySystemComponent() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

// ============== Core Accessors ==============
ACPP_PlayerStateBase* ACPP_CharacterBase::GetPlayerStateBase() const
{
    return Cast<ACPP_PlayerStateBase>(GetPlayerState());
}

UCPP_PlayerAttributeSet* ACPP_CharacterBase::GetPlayerAttributeSet() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->PlayerAttributeSet : nullptr;
}

UCPP_CharacterMovementComponent* ACPP_CharacterBase::GetCustomMovementComponent() const
{
    return Cast<UCPP_CharacterMovementComponent>(GetCharacterMovement());
}

// ============== Attribute Helpers - Delegate to PlayerState ==============
float ACPP_CharacterBase::GetHealth() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetHealth() : 0.0f;
}

float ACPP_CharacterBase::GetMaxHealth() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetMaxHealth() : 0.0f;
}

float ACPP_CharacterBase::GetMaxSpeed() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetMaxSpeed() : 0.0f;
}

float ACPP_CharacterBase::GetMaxSpeedCrouch() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetMaxSpeedCrouch() : 0.0f;
}

float ACPP_CharacterBase::GetHealthPercentage() const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetHealthPercentage() : 0.0f;
}

bool ACPP_CharacterBase::IsLowHealth(float Threshold) const
{
    const ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->IsLowHealth(Threshold) : false;
}

// ============== Combat System ==============
void ACPP_CharacterBase::ApplyDamageToSelf(float DamageAmount)
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    if (PS && DamageAmount > 0.0f)
    {
        PS->ApplyHealthChange(-DamageAmount);
    }
}

void ACPP_CharacterBase::ApplyHealingToSelf(float HealingAmount)
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    if (PS && HealingAmount > 0.0f)
    {
        PS->ApplyHealthChange(HealingAmount);
    }
}

// ============== Status Management ==============
void ACPP_CharacterBase::HandleDeath()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;

    // 移動停止
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->StopMovementImmediately();
        MovementComp->DisableMovement();
    }

    // 全アビリティキャンセル
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        ASC->CancelAllAbilities();
    }

    OnDeathEvent();
}

void ACPP_CharacterBase::HandleDamageReceived(float DamageAmount, const FGameplayTagContainer& SourceTags)
{
    if (bIsDead)
    {
        return;
    }

    OnDamageReceivedEvent(DamageAmount, SourceTags);

    if (GetHealth() <= 0.0f)
    {
        HandleDeath();
    }
}

void ACPP_CharacterBase::HandleMaxSpeedChanged(float NewSpeed)
{
    UCPP_CharacterMovementComponent* CustomMovement = GetCustomMovementComponent();
    if (!CustomMovement)
    {
        // MovementComponentが準備できていない場合、少し遅らせて再試行
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewSpeed]()
            {
                if (UCPP_CharacterMovementComponent* DelayedCustomMovement = GetCustomMovementComponent())
                {
                    DelayedCustomMovement->SetMaxSpeedWalk(NewSpeed);
                    DelayedCustomMovement->MaxWalkSpeed = NewSpeed; // 直接設定も追加
                    DelayedCustomMovement->UpdateDirectionalSpeed();
                }
            }, 0.1f, false);
        return;
    }
    CustomMovement->SetMaxSpeedWalk(NewSpeed);
    CustomMovement->MaxWalkSpeed = NewSpeed; // MaxWalkSpeedも直接設定
    CustomMovement->UpdateDirectionalSpeed();
}

void ACPP_CharacterBase::HandleMaxSpeedCrouchChanged(float NewSpeedCrouch)
{
    // しゃがみ速度変更時は即座に更新
    UCPP_CharacterMovementComponent* CustomMovement = GetCustomMovementComponent();
    if (CustomMovement && CustomMovement->IsCrouching())
    {
        CustomMovement->UpdateDirectionalSpeed();
    }
}

// ============== Sliding System Implementation ==============
void ACPP_CharacterBase::OnSlidingStarted()
{
    // C++側での処理（必要に応じて）

    // Blueprintイベントを呼び出し
    OnSlidingStartedEvent();

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("Character: Sliding Started"));
#endif
}

void ACPP_CharacterBase::OnSlidingEnded()
{
    // C++側での処理（必要に応じて）

    // Blueprintイベントを呼び出し
    OnSlidingEndedEvent();

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("Character: Sliding Ended"));
#endif
}

bool ACPP_CharacterBase::IsSliding() const
{
    if (const UCPP_CharacterMovementComponent* CustomMovement = GetCustomMovementComponent())
    {
        return CustomMovement->IsSliding();
    }
    return false;
}

void ACPP_CharacterBase::ForceStandUp()
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
