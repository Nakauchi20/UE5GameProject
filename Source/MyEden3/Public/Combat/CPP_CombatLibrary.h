// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CPP_CombatLibrary.generated.h"

/**
 * 戦闘用の静的関数ライブラリ
 * どのActorからでも呼び出せる
 */
UCLASS()
class MYEDEN3_API UCPP_CombatLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * ダメージを与える
     * どのActorからでも呼び出せる静的関数
     *
     * @param TargetActor ダメージを受けるActor
     * @param DamageAmount ダメージ量
     * @param DamageCauser ダメージを与えたActor（オプション）
     */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    static void ApplyDamage(
        AActor* TargetActor,
        float DamageAmount,
        AActor* DamageCauser = nullptr
    );

    /**
     * 回復を適用
     * どのActorからでも呼び出せる静的関数
     *
     * @param TargetActor 回復を受けるActor
     * @param HealAmount 回復量
     * @param Healer 回復を与えたActor（オプション）
     */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    static void ApplyHealing(
        AActor* TargetActor,
        float HealAmount,
        AActor* Healer = nullptr
    );

    /**
     * 爆発ダメージを適用
     * どのActorからでも呼び出せる静的関数
     *
     * @param WorldContext ワールドコンテキスト（通常は Self）
     * @param ExplosionCenter 爆発の中心位置
     * @param ExplosionRadius 爆発範囲
     * @param MaxDamage 最大ダメージ（中心でのダメージ）
     * @param DamageCauser ダメージを与えたActor（オプション）
     * @param bCheckLineOfSight 障害物判定を行うか
     * @param bDebugDraw デバッグ描画を有効にするか
     */
    UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
    static void ApplyExplosionDamage(
        UObject* WorldContextObject,
        FVector ExplosionCenter,
        float ExplosionRadius,
        float MaxDamage,
        AActor* DamageCauser = nullptr,
        bool bCheckLineOfSight = true,
        bool bDebugDraw = false
    );
};