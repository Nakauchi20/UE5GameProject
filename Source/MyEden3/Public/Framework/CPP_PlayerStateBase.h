#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/CPP_PlayerAttributeSet.h"
#include "CPP_PlayerStateBase.generated.h"

UCLASS()
class MYEDEN3_API ACPP_PlayerStateBase : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ACPP_PlayerStateBase();

protected:
    virtual void BeginPlay() override;

    // ネットワーク設定
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // ============== Ability System Component ==============
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
    class UAbilitySystemComponent* AbilitySystemComponent;

    // IAbilitySystemInterface implementation
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
    {
        return AbilitySystemComponent;
    }

    // ============== Attribute Set ==============
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    UCPP_PlayerAttributeSet* PlayerAttributeSet;

    // AttributeSetへの直接アクセスを提供
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    UCPP_PlayerAttributeSet* GetPlayerAttributeSet() const { return PlayerAttributeSet; }


    // ============== Default Settings ==============
    // 初期化用のGameplayEffect
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Defaults")
    TSubclassOf<UGameplayEffect> DefaultInitializeEffect;

    // 初期アビリティリスト（ここにAbility Listが表示される）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Defaults")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    // 初期エフェクトリスト（バフ・デバフなど）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Defaults")

    TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

    // ============== Initialization ==============
    // AbilitySystemの初期化（キャラクターから呼び出される）
    void InitializeAbilitySystem(APawn* InPawn);
    void ApplyDefaultStats();
    void GrantDefaultAbilities();

    // ============== Attribute Helpers ==============
    // よく使う属性のヘルパー関数
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxSpeed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    bool IsLowHealth(float Threshold = 0.25f) const;

    // ============== Ability Management ==============
    // アビリティの付与・削除
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void RemoveAbility(TSubclassOf<UGameplayAbility> AbilityClass);

    // アビリティの発動
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool TryActivateAbilityByTag(FGameplayTag AbilityTag);

    // ============== Gameplay Effect Management ==============
    // GameplayEffectの適用
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void ApplyGameplayEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.0f);

    // 直接的な属性変更（デバッグ用）
    UFUNCTION(BlueprintCallable, Category = "Abilities", meta = (CallInEditor = "true"))
    void ApplyHealthChange(float HealthChange);

protected:
    // ============== State Variables ==============
    UPROPERTY(BlueprintReadOnly, Category = "Status", Replicated)
    bool bAbilitySystemInitialized = false;

    // 現在のPawn参照（初期化後に設定）
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    APawn* CurrentPawn = nullptr;

    // ============== Blueprint Events ==============
    // AbilitySystemが初期化された時のBlueprintImplementableEvent
    UFUNCTION(BlueprintImplementableEvent, Category = "Abilities", meta = (DisplayName = "On Ability System Initialized"))
    void OnAbilitySystemInitializedEvent();

private:
    // 内部ヘルパー関数
    void SetupAbilitySystemComponent();
};