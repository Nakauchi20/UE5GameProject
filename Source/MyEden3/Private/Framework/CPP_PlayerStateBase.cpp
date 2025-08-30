// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_PlayerStateBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CPP_PlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

ACPP_PlayerStateBase::ACPP_PlayerStateBase()
{

    SetReplicateMovement(false);
    bReplicates = true;
    NetUpdateFrequency = 100.0f;

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    PlayerAttributeSet = CreateDefaultSubobject<UCPP_PlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

void ACPP_PlayerStateBase::BeginPlay()
{
    Super::BeginPlay();
}

void ACPP_PlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACPP_PlayerStateBase, bAbilitySystemInitialized);
}

void ACPP_PlayerStateBase::InitializeAbilitySystem(APawn* InPawn)
{
    if (!InPawn || !AbilitySystemComponent || bAbilitySystemInitialized)
    {
        return;
    }

    // AbilitySystemComponentの初期化
    AbilitySystemComponent->InitAbilityActorInfo(this, InPawn);
    bAbilitySystemInitialized = true;

    // サーバー側でのみ実行
    if (HasAuthority())
    {
        // 初期ステータスの適用
        ApplyDefaultStats();
        // 初期アビリティの付与
        GrantDefaultAbilities();
        // 初期エフェクトの適用
        for (auto& EffectClass : DefaultEffects)
        {
            if (EffectClass)
            {
                ApplyGameplayEffectToSelf(EffectClass);
            }
        }
    }
}

// 初期ステータスの適用
void ACPP_PlayerStateBase::ApplyDefaultStats()
{
    if (HasAuthority() && DefaultInitializeEffect && AbilitySystemComponent)
    {
        ApplyGameplayEffectToSelf(DefaultInitializeEffect);
    }
}
// 初期アビリティの付与
void ACPP_PlayerStateBase::GrantDefaultAbilities()
{
    if (!HasAuthority() || !AbilitySystemComponent)
    {
        return;
    }

    for (const auto& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            GrantAbility(AbilityClass);
        }
    }
}

// ============== Attribute Helpers ==============
float ACPP_PlayerStateBase::GetHealth() const
{
    return PlayerAttributeSet ? PlayerAttributeSet->GetHealth() : 0.0f;
}

float ACPP_PlayerStateBase::GetMaxHealth() const
{
    return PlayerAttributeSet ? PlayerAttributeSet->GetMaxHealth() : 0.0f;
}

float ACPP_PlayerStateBase::GetMaxSpeed() const
{
    return PlayerAttributeSet ? PlayerAttributeSet->GetMaxSpeed() : 0.0f;
}

float ACPP_PlayerStateBase::GetMaxSpeedCrouch() const
{
    return PlayerAttributeSet ? PlayerAttributeSet->GetMaxSpeedCrouch() : 0.0f;
}

float ACPP_PlayerStateBase::GetHealthPercentage() const
{
    const float MaxHealthValue = GetMaxHealth();
    return MaxHealthValue > 0.0f ? GetHealth() / MaxHealthValue : 0.0f;
}

bool ACPP_PlayerStateBase::IsLowHealth(float Threshold) const
{
    return GetHealthPercentage() <= Threshold;
}

// ============== Ability Management ==============
void ACPP_PlayerStateBase::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    if (HasAuthority() && AbilitySystemComponent && AbilityClass)
    {
        const FGameplayAbilitySpec AbilitySpec(AbilityClass, Level, INDEX_NONE, this);
        AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
}

bool ACPP_PlayerStateBase::TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
    return AbilitySystemComponent && AbilityClass ?
        AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass) : false;
}

bool ACPP_PlayerStateBase::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
    if (!AbilitySystemComponent || !AbilityTag.IsValid())
    {
        return false;
    }

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(AbilityTag);
    return AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
}

// ============== Gameplay Effect Management ==============
void ACPP_PlayerStateBase::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
    if (!EffectClass || !AbilitySystemComponent)
    {
        return;
    }

    const FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
        EffectClass, Level, Context
    );

    if (SpecHandle.IsValid())
    {
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void ACPP_PlayerStateBase::ApplyHealthChange(float HealthChange)
{
    if (AbilitySystemComponent && PlayerAttributeSet)
    {
        AbilitySystemComponent->ApplyModToAttribute(PlayerAttributeSet->GetHealthAttribute(), EGameplayModOp::Additive, HealthChange);
    }
}
