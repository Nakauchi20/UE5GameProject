// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CPP_GE_Damage.h"
#include "GAS/CPP_PlayerAttributeSet.h"

UCPP_GE_Damage::UCPP_GE_Damage()
{
    // このGEは即座に適用され瞬間的な効果を持つ
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // Health属性にダメージを与えるModifier
    FGameplayModifierInfo HealthDamageMod;
    HealthDamageMod.Attribute = UCPP_PlayerAttributeSet::GetHealthAttribute();
    HealthDamageMod.ModifierOp = EGameplayModOp::Additive; // 負の値を加算してダメージを表現

    // SetByCallerを使用してダメージ量を動的に設定
    FSetByCallerFloat DamageSetByCaller;
    DamageSetByCaller.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
    HealthDamageMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageSetByCaller);

    Modifiers.Add(HealthDamageMod);

    // ダメージタグを追加（オプション）
    FGameplayTagContainer DamageTags;
    DamageTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage")));
    InheritableOwnedTagsContainer.Added = DamageTags;
}