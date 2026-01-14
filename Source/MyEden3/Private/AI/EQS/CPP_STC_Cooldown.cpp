// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/EQS/CPP_STC_Cooldown.h"
#include "AI/EQS/CPP_STT_Cooldown.h"
#include "StateTreeExecutionContext.h"

TMap<FName, double>* UCPP_STC_Cooldown::GetCooldownEndTimes(const FStateTreeExecutionContext& Context) const
{
	// StateTreeを所有しているActorを取得
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return nullptr;
	}

	// Taskのマップを参照（friend宣言により直接アクセス可能）
	return UCPP_STT_Cooldown::CooldownEndTimesMap.Find(OwnerActor);
}

bool UCPP_STC_Cooldown::TestCondition(FStateTreeExecutionContext& Context) const
{
	// CooldownIDが設定されていない場合は常にtrue
	if (CooldownID.IsNone())
	{
		return true;
	}

	// 現在時刻を取得
	const double CurrentTime = Context.GetWorld()->GetTimeSeconds();

	// このAI個体のクールタイムマップを取得
	TMap<FName, double>* CooldownEndTimes = GetCooldownEndTimes(Context);

	// クールタイムマップが存在しない（一度も使われていない）場合はtrue
	if (!CooldownEndTimes)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("[Condition] %s: No timer map, returning true"),
			*CooldownID.ToString());*/
		return true;
	}

	// クールタイムの終了時刻を取得
	const double* EndTimePtr = CooldownEndTimes->Find(CooldownID);

	// クールタイムが存在しない、または現在時刻が終了時刻を過ぎているならtrue
	if (!EndTimePtr || CurrentTime >= *EndTimePtr)
	{
		const double RemainingTime = EndTimePtr ? (*EndTimePtr - CurrentTime) : 0.0;
		/*UE_LOG(LogTemp, Warning, TEXT("[Condition] %s: Available (%.2fs past end), returning true"),
			*CooldownID.ToString(), -RemainingTime);*/
		return true;
	}

	// クールタイム中はfalse
	const double RemainingTime = *EndTimePtr - CurrentTime;
	/*UE_LOG(LogTemp, Warning, TEXT("[Condition] %s: On cooldown (%.2fs remaining), returning false"),
		*CooldownID.ToString(), RemainingTime);*/
	return false;
}