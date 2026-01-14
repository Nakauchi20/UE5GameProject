// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CPP_CharacterBase.h"
#include "CPP_PlayerCharacter.generated.h"

// 前方宣言
class ACPP_PlayerStateBase;
class ACPP_WeaponBase;

/**
 * プレイヤー専用のキャラクタークラス
 * PlayerStateを使用してAbilitySystemを管理
 */
UCLASS()
class MYEDEN3_API ACPP_PlayerCharacter : public ACPP_CharacterBase
{
    GENERATED_BODY()

public:
    ACPP_PlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

public:
    // ============== IAbilitySystemInterface ==============
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // ============== Core Accessors ==============
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerState")
    ACPP_PlayerStateBase* GetPlayerStateBase() const;

    virtual UCPP_PlayerAttributeSet* GetPlayerAttributeSet() const override;

    // ============== Weapon System ==============
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    ACPP_WeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void SetCurrentWeapon(ACPP_WeaponBase* NewWeapon) { CurrentWeapon = NewWeapon; }

    // ============== Sliding System ==============
    UFUNCTION(BlueprintCallable, Category = "Sliding")
    void OnSlidingStarted();

    UFUNCTION(BlueprintCallable, Category = "Sliding")
    void OnSlidingEnded();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sliding")
    bool IsSliding() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ForceStandUp();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bAbilitySystemInitialized = false;

    // ============== Weapon ==============
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
    ACPP_WeaponBase* CurrentWeapon;

    // ============== Blueprint Events ==============
    UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
    void OnAbilitySystemInitializedEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Sliding")
    void OnSlidingStartedEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Sliding")
    void OnSlidingEndedEvent();

private:
    void InitializeAbilitySystem();
};