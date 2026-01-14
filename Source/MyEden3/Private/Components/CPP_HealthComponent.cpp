// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/CPP_HealthComponent.h"
#include "Engine/World.h"

UCPP_HealthComponent::UCPP_HealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCPP_HealthComponent::BeginPlay()
{
    Super::BeginPlay();
    // 初期化
    CurrentHealth = MaxHealth;
    bIsDead = false;
}

void UCPP_HealthComponent::ReceiveDamage(float DamageAmount, AActor* DamageCauser)
{
    // ダメージを受けられない、または既に死亡している場合
    if (!bCanTakeDamage || bIsDead || DamageAmount <= 0.0f)
    {
        return;
    }

    // HP減少
    const float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    const float ActualDamage = OldHealth - CurrentHealth;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    AActor* Owner = GetOwner();
    FString OwnerName = Owner ? Owner->GetName() : TEXT("Unknown");
    FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
    UE_LOG(LogTemp, Log, TEXT("%s (HealthComponent) received %.1f damage from %s - HP: %.1f/%.1f"),
        *OwnerName, ActualDamage, *CauserName, CurrentHealth, MaxHealth);
#endif

    // HP変化イベント
    OnHealthChangedEvent.Broadcast(ActualDamage, CurrentHealth, GetHealthPercentage());

    // 死亡判定
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        bIsDead = true;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        // ★ 変更: 既に宣言済みなので再宣言しない ★
        UE_LOG(LogTemp, Log, TEXT("%s (HealthComponent) destroyed"), *OwnerName);
#endif

        // 死亡イベント
        OnDeathEvent.Broadcast(DamageCauser);
    }
}

float UCPP_HealthComponent::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

void UCPP_HealthComponent::Heal(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.0f)
    {
        return;
    }

    const float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    const float ActualHeal = CurrentHealth - OldHealth;

    if (ActualHeal > 0.0f)
    {
        // HP変化イベント（回復は負のダメージとして扱う）
        OnHealthChangedEvent.Broadcast(-ActualHeal, CurrentHealth, GetHealthPercentage());

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        AActor* Owner = GetOwner();
        FString OwnerName = Owner ? Owner->GetName() : TEXT("Unknown");
        UE_LOG(LogTemp, Log, TEXT("%s (HealthComponent) healed %.1f - HP: %.1f/%.1f"),
            *OwnerName, ActualHeal, CurrentHealth, MaxHealth);
#endif
    }
}