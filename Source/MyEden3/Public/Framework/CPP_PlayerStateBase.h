#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "CPP_PlayerStateBase.generated.h"

// ëOï˚êÈåæ
class UAbilitySystemComponent;
class UCPP_PlayerAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

UCLASS()
class MYEDEN3_API ACPP_PlayerStateBase : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ACPP_PlayerStateBase();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // ============== Core Components ==============
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    UCPP_PlayerAttributeSet* PlayerAttributeSet;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    // ============== Configuration ==============
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Defaults")
    TSubclassOf<UGameplayEffect> DefaultInitializeEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Defaults")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Defaults")
    TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

    // ============== Public Interface ==============
    void InitializeAbilitySystem(APawn* InPawn);

    // Attribute accessors
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxSpeed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetMaxSpeedCrouch() const;

    // Utility functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
    bool IsLowHealth(float Threshold = 0.25f) const;

    // Ability management
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);


    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool TryActivateAbilityByTag(FGameplayTag AbilityTag);

    // GameplayEffect management
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Abilities", meta = (CallInEditor = "true"))
    void ApplyHealthChange(float HealthChange);


protected:
    UPROPERTY(BlueprintReadOnly, Category = "Status", Replicated)
    bool bAbilitySystemInitialized = false;

private:
    void ApplyDefaultStats();
    void GrantDefaultAbilities();
};