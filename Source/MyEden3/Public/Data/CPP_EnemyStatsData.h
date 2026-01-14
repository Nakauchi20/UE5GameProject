// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CPP_EnemyStatsData.generated.h"

/**
 * 敵キャラクターのステータスデータ
 * DataTableで管理するための構造体
 */
USTRUCT(BlueprintType)
struct FEnemyStatsData : public FTableRowBase
{
    GENERATED_BODY()

    // 敵の識別名（例: "Goblin", "Orc", "Dragon"）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FName EnemyID;

    // 敵の表示名
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText DisplayName;

    // ============== 基本ステータス ==============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxSpeed = 400.0f;

    // ============== オプション（将来の拡張用） ==============

    // 攻撃力（将来的に追加する場合）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Optional")
    float AttackPower = 10.0f;


    // コンストラクタ
    FEnemyStatsData()
        : EnemyID(NAME_None)
        , DisplayName(FText::FromString("Enemy"))
        , MaxHealth(100.0f)
        , MaxSpeed(400.0f)
        , AttackPower(10.0f)
    {
    }
};
