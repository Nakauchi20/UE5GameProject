#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "CPP_CharacterBase.generated.h"

// ëOï˚êÈåæ
class ACPP_PlayerStateBase;
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
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

public:

    // ============== IAbilitySystemInterface ==============
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // ============== Core Accessors ==============
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerState")
    ACPP_PlayerStateBase* GetPlayerStateBase() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    UCPP_PlayerAttributeSet* GetPlayerAttributeSet() const;

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
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToSelf(float DamageAmount);

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

    // ============== Sliding System ==============
    UFUNCTION(BlueprintCallable, Category = "Sliding")
    void OnSlidingStarted();

    UFUNCTION(BlueprintCallable, Category = "Sliding")
    void OnSlidingEnded();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sliding")
    bool IsSliding() const;

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bAbilitySystemInitialized = false;

    // ============== Blueprint Events ==============
    UFUNCTION(BlueprintImplementableEvent, Category = "Status")
    void OnDeathEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Status")
    void OnDamageReceivedEvent(float DamageAmount, const FGameplayTagContainer& SourceTags);

    UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
    void OnHealthChangedEvent(float OldHealth, float NewHealth);

    UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
    void OnAbilitySystemInitializedEvent();

    // ============== Sliding Blueprint Events ==============
    UFUNCTION(BlueprintImplementableEvent, Category = "Sliding")
    void OnSlidingStartedEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Sliding")
    void OnSlidingEndedEvent();

private:

    void InitializeAbilitySystem();
};