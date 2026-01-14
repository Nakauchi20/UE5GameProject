#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "CPP_CharacterBase.generated.h"

// 前方宣言
class UCPP_PlayerAttributeSet;
class UCPP_CharacterMovementComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;


UCLASS(BlueprintType, Blueprintable)
class MYEDEN3_API ACPP_CharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:

    ACPP_CharacterBase(const FObjectInitializer& ObjectInitializer);

protected:

    virtual void BeginPlay() override;

public:

    // ============== IAbilitySystemInterface ==============
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // ============== Core Accessors ==============
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    virtual UCPP_PlayerAttributeSet* GetPlayerAttributeSet() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    UCPP_CharacterMovementComponent* GetCustomMovementComponent() const;

    // ============== Attribute Helpers ==============
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxSpeed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxSpeedCrouch() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    bool IsLowHealth(float Threshold = 0.25f) const;

    // ============== Combat System ==============

    // ダメージを受ける（内部処理用）
    // 外部から呼ぶ場合は UCPP_CombatLibrary::ApplyDamage を使用してください
    void ReceiveDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyHealingToSelf(float HealingAmount);

    // ============== Status Management ==============
    UFUNCTION(BlueprintCallable, Category = "Status")
    virtual void HandleDeath();

    UFUNCTION(BlueprintCallable, Category = "Status")
    virtual void HandleDamageReceived(float DamageAmount, const FGameplayTagContainer& SourceTags);

    // Movement speed change handlers - called by AttributeSet
    void HandleMaxSpeedChanged(float NewSpeed);
    void HandleMaxSpeedCrouchChanged(float NewSpeedCrouch);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
    bool IsAlive() const { return !bIsDead; }

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsDead = false;

    // ============== Death Animation & Ragdoll ==============
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* DeathMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Death")
    float RagdollDuration = 3.0f;

    FTimerHandle DestroyTimerHandle;
    FTimerHandle DeathMontageTimeoutHandle;

    UFUNCTION()
    void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnDeathMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    void StartRagdoll();

    // ============== Blueprint Events ==============
    UFUNCTION(BlueprintImplementableEvent, Category = "Status")
    void OnDeathEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Status")
    void OnDamageReceivedEvent(float DamageAmount, const FGameplayTagContainer& SourceTags);

    UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
    void OnHealthChangedEvent(float OldHealth, float NewHealth);
};