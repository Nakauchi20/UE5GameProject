#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Framework/CPP_PlayerStateBase.h"
#include "CPP_CharacterBase.generated.h"


UCLASS(BlueprintType, Blueprintable)
class MYEDEN3_API ACPP_CharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:

    ACPP_CharacterBase();

protected:

    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    // 能力システムの初期化
    void InitializeAbilitySystem();

public:
    // ============== IAbilitySystemInterface ==============
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // ============== PlayerState Helpers ==============
    // PlayerStateへの安全なアクセス
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerState")
    ACPP_PlayerStateBase* GetPlayerStateBase() const;

    // AttributeSetへのアクセス
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    class UCPP_PlayerAttributeSet* GetPlayerAttributeSet() const;

    // ============== Attribute Helpers ==============
    // 属性値の取得（PlayerState経由）
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

    // ============== Combat System ==============
    // ダメージ適用
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToSelf(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToTarget(AActor* TargetActor, float DamageAmount);

    // 回復適用
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyHealingToSelf(float HealingAmount);

    // ============== Ability System Helpers ==============
    // アビリティの発動
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool TryActivateAbilityByTag(FGameplayTag AbilityTag);

    // GameplayEffect適用
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.0f);

    // ============== Status Management ==============
    // 死亡処理
    UFUNCTION(BlueprintCallable, Category = "Status")
    virtual void HandleDeath();

    // ダメージ受信時の処理
    UFUNCTION(BlueprintCallable, Category = "Status")
    virtual void HandleDamageReceived(float DamageAmount, const FGameplayTagContainer& SourceTags);

    // 移動速度変更時の処理
    virtual void HandleMaxSpeedChanged(float NewSpeed);

    // ステータス確認
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
    bool IsAlive() const { return !bIsDead; }

protected:
    // ============== State Variables ==============
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bAbilitySystemInitialized = false;

    // ============== Blueprint Events ==============
    // 死亡時イベント
    UFUNCTION(BlueprintImplementableEvent, Category = "Status", meta = (DisplayName = "On Death"))
    void OnDeathEvent();

    // ダメージ受信時イベント
    UFUNCTION(BlueprintImplementableEvent, Category = "Status", meta = (DisplayName = "On Damage Received"))
    void OnDamageReceivedEvent(float DamageAmount, const FGameplayTagContainer& SourceTags);

    // 体力変更時イベント
    UFUNCTION(BlueprintImplementableEvent, Category = "Attributes", meta = (DisplayName = "On Health Changed"))
    void OnHealthChangedEvent(float OldHealth, float NewHealth);

    // AbilitySystem初期化完了時イベント
    UFUNCTION(BlueprintImplementableEvent, Category = "Abilities", meta = (DisplayName = "On Ability System Initialized"))
    void OnAbilitySystemInitializedEvent();
};