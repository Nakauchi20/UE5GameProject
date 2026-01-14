// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CPP_CharacterBase.h"
#include "CPP_EnemyCharacter.generated.h"

// 前方宣言
class UAbilitySystemComponent;
class UCPP_PlayerAttributeSet;
class UCPP_CharacterMovementComponent;
class UDataTable;
class UStateTreeComponent;
struct FEnemyStatsData;

/**
 * 敵キャラクタークラス
 * AbilitySystemComponentとAttributeSetを自身に持つ
 */
UCLASS()
class MYEDEN3_API ACPP_EnemyCharacter : public ACPP_CharacterBase
{
    GENERATED_BODY()

public:
    ACPP_EnemyCharacter(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;

public:
    // ============== IAbilitySystemInterface ==============
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // ============== Core Accessors ==============
    virtual UCPP_PlayerAttributeSet* GetPlayerAttributeSet() const override;

    // ============== Death Handling ==============
    virtual void HandleDeath() override;

    // ============== Stats Configuration ==============
    // 敵のステータスID（DataTableの行名）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
    FName EnemyStatsID = "Goblin";

    // 敵ステータス用DataTable参照
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
    TObjectPtr<UDataTable> EnemyStatsTable;

    // DataTableからステータスを読み込んで適用
    UFUNCTION(BlueprintCallable, Category = "Enemy Stats")
    bool LoadStatsFromDataTable();

    // 直接ステータスを設定（DataTableを使わない場合）
    UFUNCTION(BlueprintCallable, Category = "Enemy Stats")
    void SetInitialStats(float InMaxHealth, float InMaxSpeed);

    // ============== Debug Functions ==============

    // デバッグ用: ステータス情報を取得
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetDebugStatsInfo() const;

    // デバッグ用: 画面にステータスを表示
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DisplayStatsOnScreen(float Duration = 5.0f) const;

protected:
    // ============== Components ==============
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
    TObjectPtr<UCPP_PlayerAttributeSet> AttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UStateTreeComponent> StateTreeComponent;

    // ============== Blueprint Events ==============
    UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
    void OnAbilitySystemInitializedEvent();

private:
    void InitializeAbilitySystem();
};