// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CPP_GA_Base.h"
#include "GameplayTagContainer.h"
#include "CPP_GA_Damage.generated.h"

/**
 * ダメージを与えるためのGameplay Ability
 */
UCLASS()
class MYEDEN3_API UCPP_GA_Damage : public UCPP_GA_Base
{
	GENERATED_BODY()
	
public:
	UCPP_GA_Damage();

protected:
	// Ability実行時の処理
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// ダメージ量（Blueprintで設定可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageAmount;

	// ダメージ効果のGameplayEffect（Blueprintで設定）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<class UGameplayEffect> DamageGameplayEffect;

	// ターゲットにダメージを与える関数
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyDamageToTarget(AActor* TargetActor, float Damage);
};
