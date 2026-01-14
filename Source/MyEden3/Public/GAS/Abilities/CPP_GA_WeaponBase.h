// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPP_GA_WeaponBase.generated.h"

// 前方宣言
class ACPP_WeaponBase;
class ACPP_PlayerCharacter;

/**
 * 武器アビリティの基底クラス
 * GA_FireとGA_ReloadのBPで継承して使用
 */
UCLASS()
class MYEDEN3_API UCPP_GA_WeaponBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPP_GA_WeaponBase();

protected:
	// === ヘルパー関数 ===

	/** 現在装備中の武器を取得 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|Weapon")
	ACPP_WeaponBase* GetCurrentWeapon() const;

	/** プレイヤーキャラクターを取得 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|Weapon")
	ACPP_PlayerCharacter* GetPlayerCharacter() const;

	/** 武器が有効か確認 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|Weapon")
	bool IsWeaponValid() const;
};