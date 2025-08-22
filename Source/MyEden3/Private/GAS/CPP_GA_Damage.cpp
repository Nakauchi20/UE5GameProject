// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CPP_GA_Damage.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Characters/CPP_CharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h" 

UCPP_GA_Damage::UCPP_GA_Damage()
{
    // デフォルトダメージ量
    DamageAmount = 25.0f;

    // Abilityの基本設定
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCPP_GA_Damage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // 親クラスの処理を呼び出し
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // アビリティの実行を確認
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ターゲットの取得（例：プレイヤー自身、または指定されたターゲット）
    AActor* TargetActor = GetAvatarActorFromActorInfo();

    if (TargetActor)
    {
        ApplyDamageToTarget(TargetActor, DamageAmount);
    }

    // アビリティ終了
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UCPP_GA_Damage::ApplyDamageToTarget(AActor* TargetActor, float Damage)
{
    if (!TargetActor)
    {
        return;
    }

    // ターゲットのAbility System Componentを取得
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC)
    {
        return;
    }

    // GameplayEffectが設定されている場合はそれを使用
    if (DamageGameplayEffect)
    {
        // GameplayEffectSpecを作成
        FGameplayEffectContextHandle EffectContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
        EffectContextHandle.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageGameplayEffect, GetAbilityLevel(), EffectContextHandle);

        if (SpecHandle.IsValid())
        {
            // ダメージ量を設定（GameplayEffectに応じて調整）
            SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);

            // エフェクトを適用
            GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        }
    }
    else
    {
        // 直接ダメージを適用する場合の処理
        // この例では簡単な実装として、Health属性を直接変更
        UE_LOG(LogTemp, Warning, TEXT("DamageGameplayEffect is not set. Cannot apply damage."));
    }
}