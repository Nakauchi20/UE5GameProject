// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_HealthComponent.generated.h"

// ============== デリゲート宣言（UCLASSの前に書く） ==============
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedSignature, float, DamageAmount, float, NewHealth, float, NewHealthPercentage);

/**
 * ヘルスコンポーネント
 * 任意のActorに追加してダメージを受けられるようにする
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYEDEN3_API UCPP_HealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCPP_HealthComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ============== Properties ==============

    // 最大HP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.0f;

    // 現在のHP
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    // ダメージを受けられるか
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bCanTakeDamage = true;

    // ============== Events ==============

    // 死亡時イベント
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnDeathSignature OnDeathEvent;

    // HP変化時イベント
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChangedSignature OnHealthChangedEvent;

    // ============== Functions ==============

    // ダメージを受ける
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ReceiveDamage(float DamageAmount, AActor* DamageCauser);

    // 生存確認
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    bool IsAlive() const { return CurrentHealth > 0.0f; }

    // HP取得
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetHealth() const { return CurrentHealth; }

    // 最大HP取得
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetMaxHealth() const { return MaxHealth; }

    // HP割合取得
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    // HPを回復
    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

protected:
    // 死亡済みフラグ
    bool bIsDead = false;
};