// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CPP_CharacterBase.h"
#include "GAS/CPP_PlayerAttributeSet.h"
#include "Characters/CPP_CharacterMovementComponent.h"
#include "Combat/CPP_CombatLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Framework/CPP_GameProgressSubsystem.h"


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

// ============== IAbilitySystemInterface ==============
UAbilitySystemComponent* ACPP_CharacterBase::GetAbilitySystemComponent() const
{
    // サブクラスでオーバーライドされる
    return nullptr;
}

// ============== Core Accessors ==============
UCPP_PlayerAttributeSet* ACPP_CharacterBase::GetPlayerAttributeSet() const
{
    // サブクラスでオーバーライドされる
    return nullptr;
}

UCPP_CharacterMovementComponent* ACPP_CharacterBase::GetCustomMovementComponent() const
{
    return Cast<UCPP_CharacterMovementComponent>(GetCharacterMovement());
}

// ============== Attribute Helpers ==============
float ACPP_CharacterBase::GetHealth() const
{
    const UCPP_PlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
    return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

float ACPP_CharacterBase::GetMaxHealth() const
{
    const UCPP_PlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
    return AttributeSet ? AttributeSet->GetMaxHealth() : 0.0f;
}

float ACPP_CharacterBase::GetMaxSpeed() const
{
    const UCPP_PlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
    return AttributeSet ? AttributeSet->GetMaxSpeed() : 0.0f;
}

float ACPP_CharacterBase::GetMaxSpeedCrouch() const
{
    const UCPP_PlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
    return AttributeSet ? AttributeSet->GetMaxSpeedCrouch() : 0.0f;
}

float ACPP_CharacterBase::GetHealthPercentage() const
{
    const float MaxHP = GetMaxHealth();
    return MaxHP > 0.0f ? (GetHealth() / MaxHP) : 0.0f;
}

bool ACPP_CharacterBase::IsLowHealth(float Threshold) const
{
    return GetHealthPercentage() <= Threshold;
}

// ============== Combat System ==============
void ACPP_CharacterBase::ReceiveDamage(float DamageAmount, AActor* DamageCauser)
{
    if (DamageAmount <= 0.0f || bIsDead)
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        if (bIsDead)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ReceiveDamage] %s is already dead, ignoring damage"), *GetName());
        }
#endif
        return;
    }

    // HP減少
    const float CurrentHealth = GetHealth();
    const float NewHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (UCPP_PlayerAttributeSet* AttributeSet = GetPlayerAttributeSet())
    {
        AttributeSet->SetHealth(NewHealth);
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[ReceiveDamage] %s received %.1f damage - HP: %.1f/%.1f"),
        *GetName(), DamageAmount, GetHealth(), GetMaxHealth());
#endif

    // ダメージ受け取り処理
    FGameplayTagContainer EmptyTags;
    HandleDamageReceived(DamageAmount, EmptyTags);
}
void ACPP_CharacterBase::ApplyHealingToSelf(float HealingAmount)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC || HealingAmount <= 0.0f)
    {
        return;
    }

    const float CurrentHealth = GetHealth();
    const float MaxHP = GetMaxHealth();
    const float NewHealth = FMath::Min(MaxHP, CurrentHealth + HealingAmount);

    if (UCPP_PlayerAttributeSet* AttributeSet = GetPlayerAttributeSet())
    {
        AttributeSet->SetHealth(NewHealth);
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

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] %s is dying"), *GetName());
#endif

    // ★ 追加：Subsystemに死亡を通知（プレイヤーの場合のみ）
    if (GetWorld() && IsPlayerControlled())
    {
        UCPP_GameProgressSubsystem* GameProgress = GetWorld()->GetSubsystem<UCPP_GameProgressSubsystem>();
        if (GameProgress)
        {
            GameProgress->NotifyPlayerDied();
        }
    }

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

    // 死亡アニメーション再生
    if (DeathMontage)
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] DeathMontage found, attempting to play"));
#endif

        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] AnimInstance valid, binding delegate"));
#endif

            // ブレンドアウト開始時のデリゲートをバインド
            FOnMontageBlendingOutStarted BlendingOutDelegate;
            BlendingOutDelegate.BindUObject(this, &ACPP_CharacterBase::OnDeathMontageBlendingOut);
            AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, DeathMontage);

            // Montage終了時のデリゲートもバインド（バックアップ）
            FOnMontageEnded MontageEndedDelegate;
            MontageEndedDelegate.BindUObject(this, &ACPP_CharacterBase::OnDeathMontageEnded);
            AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DeathMontage);

            // Montage再生
            float PlayRate = AnimInstance->Montage_Play(DeathMontage, 1.0f);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] Montage play returned: %f"), PlayRate);
#endif

            // モンタージュの95%の時点でRagdollにする（ほぼ倒れ切った時点）
            const float MontageLength = DeathMontage->GetPlayLength();
            const float RagdollDelay = MontageLength * 0.95f; // 95%の時点

            GetWorldTimerManager().SetTimer(
                DeathMontageTimeoutHandle,
                [this]()
                {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
                    UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] Montage 95%% reached for %s - starting ragdoll"), *GetName());
#endif
                    StartRagdoll();
                },
                RagdollDelay,
                false
            );

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] Ragdoll timer set for %f seconds (95%% of montage)"), RagdollDelay);
#endif
        }
        else
        {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] AnimInstance is null, starting ragdoll immediately"));
#endif
            StartRagdoll();
        }
    }
    else
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("[HandleDeath] No DeathMontage set, starting ragdoll immediately"));
#endif
        StartRagdoll();
    }

    OnDeathEvent();
}

void ACPP_CharacterBase::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[OnDeathMontageEnded] Called for %s - Interrupted: %s"),
        *GetName(), bInterrupted ? TEXT("TRUE") : TEXT("FALSE"));
#endif

    // タイムアウトタイマーをクリア
    GetWorldTimerManager().ClearTimer(DeathMontageTimeoutHandle);

    // まだRagdollになっていなければ開始
    if (GetCapsuleComponent()->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
    {
        StartRagdoll();
    }
}

void ACPP_CharacterBase::OnDeathMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[OnDeathMontageBlendingOut] Called for %s - Interrupted: %s"),
        *GetName(), bInterrupted ? TEXT("TRUE") : TEXT("FALSE"));
#endif

    // タイムアウトタイマーをクリア
    GetWorldTimerManager().ClearTimer(DeathMontageTimeoutHandle);

    // ブレンドアウト開始時点でRagdollを開始
    StartRagdoll();
}

void ACPP_CharacterBase::StartRagdoll()
{
    // 既にRagdoll化されている場合は何もしない
    if (GetCapsuleComponent()->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("[StartRagdoll] %s already ragdolled, skipping"), *GetName());
#endif
        return;
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[StartRagdoll] Starting ragdoll for %s"), *GetName());
#endif

    // タイムアウトタイマーをクリア（念のため）
    GetWorldTimerManager().ClearTimer(DeathMontageTimeoutHandle);

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        // 現在のアニメーションポーズを保存
        UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
        if (AnimInstance)
        {
            // アニメーションを停止して最終フレームで固定
            AnimInstance->Montage_Stop(0.0f);  // ブレンド時間0で即座に停止
        }

        // 物理シミュレーションを有効化する前に、現在の姿勢を物理ボディに反映
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
        MeshComp->SetSimulatePhysics(true);

        // 物理シミュレーション有効化後、全ボディの速度を0にリセット
        MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
        MeshComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector, false, NAME_None);

        // 重力の影響を受けるように設定
        MeshComp->SetEnableGravity(true);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("[StartRagdoll] Mesh ragdoll enabled with zero velocity"));
#endif
    }

    // コリジョン無効化
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 指定時間後にアクター破棄
    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle,
        [this]()
        {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("[StartRagdoll] Destroying actor %s"), *GetName());
#endif
            Destroy();
        },
        RagdollDuration,
        false
    );

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[StartRagdoll] Destroy timer set for %f seconds"), RagdollDuration);
#endif
}

void ACPP_CharacterBase::HandleDamageReceived(float DamageAmount, const FGameplayTagContainer& SourceTags)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[HandleDamageReceived] %s - Damage: %.1f, HP: %.1f, bIsDead: %s"),
        *GetName(), DamageAmount, GetHealth(), bIsDead ? TEXT("TRUE") : TEXT("FALSE"));
#endif

    if (bIsDead)
    {
        return;
    }

    OnDamageReceivedEvent(DamageAmount, SourceTags);

    if (GetHealth() <= 0.0f)
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("[HandleDamageReceived] %s health is 0, calling HandleDeath"), *GetName());
#endif
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
                    DelayedCustomMovement->MaxWalkSpeed = NewSpeed;
                    DelayedCustomMovement->UpdateDirectionalSpeed();
                }
            }, 0.1f, false);
        return;
    }
    CustomMovement->SetMaxSpeedWalk(NewSpeed);
    CustomMovement->MaxWalkSpeed = NewSpeed;
    CustomMovement->UpdateDirectionalSpeed();
}

void ACPP_CharacterBase::HandleMaxSpeedCrouchChanged(float NewSpeedCrouch)
{
    UCPP_CharacterMovementComponent* CustomMovement = GetCustomMovementComponent();
    if (CustomMovement && CustomMovement->IsCrouching())
    {
        CustomMovement->UpdateDirectionalSpeed();
    }
}