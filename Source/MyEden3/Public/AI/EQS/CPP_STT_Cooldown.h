// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "CPP_STT_Cooldown.generated.h"

/**
 * StateTreeのクールタイムTask
 * 指定されたCooldownIDでクールタイムを管理し、Wait的に動作する
 * クールタイム中の場合は即座に失敗、終了している場合はクールタイムを開始して成功
 */
UCLASS()
class MYEDEN3_API UCPP_STT_Cooldown : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	// クールタイムの識別子
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName CooldownID = NAME_None;

	// クールタイムの長さ（秒）
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float CooldownDuration = 1.0f;


private:
	// 全てのクールタイムの終了時刻を管理 (Key: AI個体のActor, Value: クールタイムマップ)
	// StateTreeインスタンス全体で共有するため、staticで管理
	// Conditionクラスからもアクセスされる
	static TMap<TWeakObjectPtr<AActor>, TMap<FName, double>> CooldownEndTimesMap;

	// 現在のAI個体のクールタイムマップを取得
	TMap<FName, double>* GetCooldownEndTimes(const FStateTreeExecutionContext& Context) const;

	// 無効なエントリをクリーンアップ
	static void CleanupInvalidEntries();

	// Conditionクラスがマップにアクセスできるようにfriend宣言
	friend class UCPP_STC_Cooldown;
};