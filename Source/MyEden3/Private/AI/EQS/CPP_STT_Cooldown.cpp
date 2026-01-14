// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/EQS/CPP_STT_Cooldown.h"
#include "StateTreeExecutionContext.h"

// staticメンバの定義
TMap<TWeakObjectPtr<AActor>, TMap<FName, double>> UCPP_STT_Cooldown::CooldownEndTimesMap;

TMap<FName, double>* UCPP_STT_Cooldown::GetCooldownEndTimes(const FStateTreeExecutionContext& Context) const
{
	// StateTreeを所有しているActorを取得（通常はAIController or Pawn）
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return nullptr;
	}
	
	// 定期的にクリーンアップ（10%の確率で実行）
	if (FMath::RandRange(0, 9) == 0)
	{
		CleanupInvalidEntries();
	}
	
	return CooldownEndTimesMap.Find(OwnerActor);
}

void UCPP_STT_Cooldown::CleanupInvalidEntries()
{
	// 無効になったエントリを削除
	for (auto It = CooldownEndTimesMap.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

EStateTreeRunStatus UCPP_STT_Cooldown::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	// CooldownIDが設定されていない場合は即座に成功
	if (CooldownID.IsNone())
	{
		return EStateTreeRunStatus::Succeeded;
	}
	
	// StateTreeを所有しているActorを取得
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	// 現在時刻を取得
	const double CurrentTime = Context.GetWorld()->GetTimeSeconds();
	
	// このAI個体のクールタイムマップを取得または作成
	TMap<FName, double>& CooldownEndTimes = CooldownEndTimesMap.FindOrAdd(OwnerActor);
	
	// クールタイムの終了時刻を取得
	double* EndTimePtr = CooldownEndTimes.Find(CooldownID);
	
	if (EndTimePtr && CurrentTime < *EndTimePtr)
	{
		// クールタイム中 → 失敗
		const double RemainingTime = *EndTimePtr - CurrentTime;
		/*UE_LOG(LogTemp, Warning, TEXT("[Task] %s is on cooldown (%.2fs remaining), Failed"),
			*CooldownID.ToString(), RemainingTime);*/
		return EStateTreeRunStatus::Failed;
	}
	
	// クールタイム終了 or 初回 → クールタイム開始（終了時刻を記録）
	const double EndTime = CurrentTime + CooldownDuration;
	CooldownEndTimes.Add(CooldownID, EndTime);
	
	/*UE_LOG(LogTemp, Warning, TEXT("[Task] %s cooldown started (%.2fs), EndTime=%.2f"),
		*CooldownID.ToString(), CooldownDuration, EndTime);*/
	
	// 即座に成功（Stateから抜けてもクールタイムは進行する）
	return EStateTreeRunStatus::Succeeded;
}