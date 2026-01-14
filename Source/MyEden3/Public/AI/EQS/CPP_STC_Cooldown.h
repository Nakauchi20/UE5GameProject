// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "CPP_STC_Cooldown.generated.h"

/**
 * StateTreeのクールタイムCondition
 * 指定されたCooldownIDのクールタイムをチェックする
 * クールタイム中: false（遷移しない）
 * クールタイム終了または未開始: true（遷移可能）
 *
 * 注意: このConditionはクールタイムの開始はしません
 * クールタイムの開始は別のTask（UCPP_STT_Cooldown）で行ってください
 */
UCLASS()
class MYEDEN3_API UCPP_STC_Cooldown : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	// クールタイムの識別子
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName CooldownID = NAME_None;

private:
	// 現在のAI個体のクールタイムマップを取得
	// Taskのマップを参照する
	TMap<FName, double>* GetCooldownEndTimes(const FStateTreeExecutionContext& Context) const;
};