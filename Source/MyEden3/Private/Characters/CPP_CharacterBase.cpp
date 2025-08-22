// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CPP_CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CPP_GE_Damage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

ACPP_CharacterBase::ACPP_CharacterBase()
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

    // PlayerStateが設定された後にAbilitySystemを初期化
    InitializeAbilitySystem();
}

void ACPP_CharacterBase::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // クライアント側でPlayerStateがレプリケートされた後に初期化
    InitializeAbilitySystem();
}

void ACPP_CharacterBase::InitializeAbilitySystem()
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    if (!PS || bAbilitySystemInitialized)
    {
        return;
    }

    // PlayerStateのAbilitySystemを初期化
    PS->InitializeAbilitySystem(this);
    bAbilitySystemInitialized = true;

    // Blueprintイベント呼び出し
    OnAbilitySystemInitializedEvent();

    UE_LOG(LogTemp, Log, TEXT("%s: Character Ability System Initialized"), *GetName());
}

// ============== IAbilitySystemInterface ==============
UAbilitySystemComponent* ACPP_CharacterBase::GetAbilitySystemComponent() const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

// ============== PlayerState Helpers ==============
ACPP_PlayerStateBase* ACPP_CharacterBase::GetPlayerStateBase() const
{
    return Cast<ACPP_PlayerStateBase>(GetPlayerState());
}

UCPP_PlayerAttributeSet* ACPP_CharacterBase::GetPlayerAttributeSet() const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetPlayerAttributeSet() : nullptr;
}

// ============== Attribute Helpers ==============
float ACPP_CharacterBase::GetHealth() const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetHealth() : 0.0f;
}

float ACPP_CharacterBase::GetMaxHealth() const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetMaxHealth() : 0.0f;
}

float ACPP_CharacterBase::GetMaxSpeed() const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetMaxSpeed() : 0.0f;
}

float ACPP_CharacterBase::GetHealthPercentage() const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->GetHealthPercentage() : 0.0f;
}

bool ACPP_CharacterBase::IsLowHealth(float Threshold) const
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->IsLowHealth(Threshold) : false;
}

// ============== Combat System ==============
void ACPP_CharacterBase::ApplyDamageToSelf(float DamageAmount)
{
    if (DamageAmount <= 0.0f)
    {
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    // ダメージ用のGameplayEffectを使用
    if (TSubclassOf<UGameplayEffect> DamageEffect = UCPP_GE_Damage::StaticClass())
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
            DamageEffect, 1.0f, Context
        );

        if (SpecHandle.IsValid())
        {
            // SetByCallerでダメージ量を設定（負の値でダメージを表現）
            SpecHandle.Data->SetSetByCallerMagnitude(
                FGameplayTag::RequestGameplayTag(FName("Data.Damage")),
                -DamageAmount
            );

            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

    // ダメージ処理イベント
    HandleDamageReceived(DamageAmount, FGameplayTagContainer());
}

void ACPP_CharacterBase::ApplyDamageToTarget(AActor* TargetActor, float DamageAmount)
{
    if (!TargetActor || DamageAmount <= 0.0f)
    {
        return;
    }

    UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

    if (!MyASC || !TargetASC)
    {
        return;
    }

    if (TSubclassOf<UGameplayEffect> DamageEffect = UCPP_GE_Damage::StaticClass())
    {
        FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
        Context.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(
            DamageEffect, 1.0f, Context
        );

        if (SpecHandle.IsValid())
        {
            SpecHandle.Data->SetSetByCallerMagnitude(
                FGameplayTag::RequestGameplayTag(FName("Data.Damage")),
                -DamageAmount
            );

            MyASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        }
    }
}

void ACPP_CharacterBase::ApplyHealingToSelf(float HealingAmount)
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    if (PS)
    {
        PS->ApplyHealthChange(HealingAmount);
    }
}

// ============== Ability System Helpers ==============
bool ACPP_CharacterBase::TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->TryActivateAbilityByClass(AbilityClass) : false;
}

bool ACPP_CharacterBase::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    return PS ? PS->TryActivateAbilityByTag(AbilityTag) : false;
}

void ACPP_CharacterBase::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
    ACPP_PlayerStateBase* PS = GetPlayerStateBase();
    if (PS)
    {
        PS->ApplyGameplayEffectToSelf(EffectClass, Level);
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

    // 移動を停止
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->StopMovementImmediately();
        MovementComp->DisableMovement();
    }

    // 全アビリティをキャンセル
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (ASC)
    {
        ASC->CancelAllAbilities();
    }

    // Blueprintイベント呼び出し
    OnDeathEvent();

    UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());
}

void ACPP_CharacterBase::HandleDamageReceived(float DamageAmount, const FGameplayTagContainer& SourceTags)
{
    if (bIsDead)
    {
        return;
    }

    // Blueprintイベント呼び出し
    OnDamageReceivedEvent(DamageAmount, SourceTags);

    // 体力が0以下になったら死亡処理
    if (GetHealth() <= 0.0f)
    {
        HandleDeath();
    }

    UE_LOG(LogTemp, Log, TEXT("%s received %.2f damage"), *GetName(), DamageAmount);
}

void ACPP_CharacterBase::HandleMaxSpeedChanged(float NewSpeed)
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = NewSpeed;
        UE_LOG(LogTemp, Log, TEXT("%s max speed changed to %.2f"), *GetName(), NewSpeed);
    }
}
