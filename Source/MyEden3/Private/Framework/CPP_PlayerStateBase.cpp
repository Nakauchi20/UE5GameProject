// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_PlayerStateBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
//#include "GAS/CPP_GE_InitializeStats.h"
//#include "UObject/ConstructorHelpers.h"

ACPP_PlayerStateBase::ACPP_PlayerStateBase()
{
    // ネットワーク設定
    SetReplicateMovement(false);
    bReplicates = true;
    NetUpdateFrequency = 100.0f;

    // Ability System Component
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // Attribute Set
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

    CurrentPawn = InPawn;

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
        UE_LOG(LogTemp, Warning, TEXT("Initializing Ability System for %s"), *InPawn->GetName());
        // 初期エフェクトの適用
        for (auto& EffectClass : DefaultEffects)
        {
            if (EffectClass)
            {
                ApplyGameplayEffectToSelf(EffectClass);
                UE_LOG(LogTemp, Warning, TEXT("Applying Default Effect: %s"), *EffectClass->GetName());
            }
        }
    }

    // Blueprintイベントの呼び出し
    //OnAbilitySystemInitializedEvent();
    UE_LOG(LogTemp, Warning, TEXT("%s: PlayerState Ability System Initialized for %s"), *GetName(), *InPawn->GetName());
}

// 初期ステータスの適用
void ACPP_PlayerStateBase::ApplyDefaultStats()
{
    if (!HasAuthority() || !DefaultInitializeEffect || !AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Cannot apply default stats"), *GetName());
        return;
    }
    ApplyGameplayEffectToSelf(DefaultInitializeEffect);
    UE_LOG(LogTemp, Log, TEXT("%s: Default stats applied"), *GetName());
}
// 初期アビリティの付与
void ACPP_PlayerStateBase::GrantDefaultAbilities()
{
    if (!HasAuthority() || !AbilitySystemComponent)
    {
        return;
    }

    for (auto& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            GrantAbility(AbilityClass);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("%s: Default abilities granted (%d abilities)"), *GetName(), DefaultAbilities.Num());
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

float ACPP_PlayerStateBase::GetHealthPercentage() const
{
    float MaxHealthValue = GetMaxHealth();
    return MaxHealthValue > 0.0f ? GetHealth() / MaxHealthValue : 0.0f;
}

bool ACPP_PlayerStateBase::IsLowHealth(float Threshold) const
{
    return GetHealthPercentage() <= Threshold;
}

// ============== Ability Management ==============
void ACPP_PlayerStateBase::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    if (!AbilitySystemComponent || !AbilityClass)
    {
        return;
    }

    if (HasAuthority())
    {
        FGameplayAbilitySpec AbilitySpec(AbilityClass, Level, INDEX_NONE, this);
        AbilitySystemComponent->GiveAbility(AbilitySpec);
        UE_LOG(LogTemp, Log, TEXT("%s: Granted ability %s"), *GetName(), *AbilityClass->GetName());
    }
}

void ACPP_PlayerStateBase::RemoveAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
    if (!AbilitySystemComponent || !AbilityClass)
    {
        return;
    }

    if (HasAuthority())
    {
        // 特定のアビリティクラスのみを削除する処理
        TArray<FGameplayAbilitySpec> AbilitiesToRemove;
        for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
        {
            if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
            {
                AbilitiesToRemove.Add(Spec);
            }
        }

        for (const FGameplayAbilitySpec& Spec : AbilitiesToRemove)
        {
            AbilitySystemComponent->ClearAbility(Spec.Handle);
        }
    }
}

bool ACPP_PlayerStateBase::TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
    if (!AbilitySystemComponent || !AbilityClass)
    {
        return false;
    }

    return AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
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

    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    Context.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
        EffectClass, Level, Context
    );

    if (SpecHandle.IsValid())
    {
        // ASC の標準関数を呼んでいる
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void ACPP_PlayerStateBase::ApplyGameplayEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
    if (!TargetActor || !EffectClass || !AbilitySystemComponent)
    {
        return;
    }

    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC)
    {
        return;
    }

    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    Context.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
        EffectClass, Level, Context
    );

    if (SpecHandle.IsValid())
    {
        AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    }
}

void ACPP_PlayerStateBase::ApplyHealthChange(float HealthChange)
{
    if (!AbilitySystemComponent || !PlayerAttributeSet)
    {
        return;
    }

    // 直接的な属性変更（デバッグ用）
    AbilitySystemComponent->ApplyModToAttribute(
        PlayerAttributeSet->GetHealthAttribute(),
        EGameplayModOp::Additive,
        HealthChange
    );

    UE_LOG(LogTemp, Log, TEXT("%s: Health changed by %.2f"), *GetName(), HealthChange);
}
