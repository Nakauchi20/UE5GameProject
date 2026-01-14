// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/CPP_EQT_IdealDistancePoint.h"
#include "GameFramework/Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

UCPP_EQT_IdealDistancePoint::UCPP_EQT_IdealDistancePoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SightFrom = UEnvQueryContext_Querier::StaticClass();// Contextを設定（視点となるプレイヤー）
    ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();// 評価対象はアクター

    // デフォルト値を設定
    IdealDistanceMin.DefaultValue = 2000.0f;
    IdealDistanceMax.DefaultValue = 3000.0f;
    MinEvaluationRange.DefaultValue = 1500.0f;
    MaxEvaluationRange.DefaultValue = 3500.0f;
}

void UCPP_EQT_IdealDistancePoint::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* QueryOwner = QueryInstance.Owner.Get();
    if (QueryOwner == nullptr)
    {
        return;
    }

    // Data Bindingから実際の値を取得
    MinEvaluationRange.BindData(QueryOwner, QueryInstance.QueryID);
    IdealDistanceMin.BindData(QueryOwner, QueryInstance.QueryID);
    IdealDistanceMax.BindData(QueryOwner, QueryInstance.QueryID);
    MaxEvaluationRange.BindData(QueryOwner, QueryInstance.QueryID);

    float MinEvalRange = MinEvaluationRange.GetValue();
    float IdealDistMin = IdealDistanceMin.GetValue();
    float IdealDistMax = IdealDistanceMax.GetValue();
    float MaxEvalRange = MaxEvaluationRange.GetValue();

    // パラメータ確認ログ追加
    UE_LOG(LogTemp, Warning, TEXT("=== EQS Parameters ==="));
    UE_LOG(LogTemp, Warning, TEXT("MinEval=%.1f, IdealMin=%.1f, IdealMax=%.1f, MaxEval=%.1f"),
        MinEvalRange, IdealDistMin, IdealDistMax, MaxEvalRange);

    // Querier（プレイヤー）を取得
    TArray<AActor*> ContextActors;
    if (!QueryInstance.PrepareContext(SightFrom, ContextActors) || ContextActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ PrepareContext failed or no context actors found."));
        return;
    }

    const AActor* QuerierActor = ContextActors[0];
    const FVector QuerierLocation = QuerierActor->GetActorLocation();

    UE_LOG(LogTemp, Warning, TEXT("Querier Location: %s"), *QuerierLocation.ToString());

    // テスト対象のアイテムを走査
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        const FVector CandidatePoint = GetItemLocation(QueryInstance, It.GetIndex());
        float Distance = FVector::Dist(QuerierLocation, CandidatePoint);
        float Score = 0.f;
        FString DebugCategory = TEXT("OutOfRange");

        // 範囲外チェックを先に
        if (Distance < MinEvalRange || Distance > MaxEvalRange)
        {
            Score = 0.f;
            DebugCategory = TEXT("OutOfRange");
        }
        // 理想範囲内(IdealDistanceMin～IdealDistanceMax)
        else if (Distance >= IdealDistMin && Distance <= IdealDistMax)
        {
            Score = 1.0f;
            DebugCategory = TEXT("IdealRange");
        }
        // 下側カーブ (MinEvalRange → IdealDistMin)
        else if (Distance < IdealDistMin)
        {
            float Range = IdealDistMin - MinEvalRange;
            if (Range > 0.f)
            {
                float NormalizedDist = (Distance - MinEvalRange) / Range;
                Score = FMath::SmoothStep(0.f, 1.f, NormalizedDist);
                DebugCategory = TEXT("LowerCurve");
            }
        }
        // 上側カーブ (IdealDistMax → MaxEvalRange)
        else  // Distance > IdealDistMax ← ここを修正
        {
            float Range = MaxEvalRange - IdealDistMax;
            if (Range > 0.f)
            {
                float NormalizedDist = (Distance - IdealDistMax) / Range;
                Score = FMath::SmoothStep(1.f, 0.f, NormalizedDist);
                DebugCategory = TEXT("UpperCurve");
            }
        }
        // 全候補のログ出力
        UE_LOG(LogTemp, Log, TEXT("[%s] Dist=%.1f, Score=%.3f"),
            *DebugCategory, Distance, Score);

        It.SetScore(TestPurpose, FilterType, Score, 0.f, 1.f);
    }
}

FText UCPP_EQT_IdealDistancePoint::GetDescriptionTitle() const
{
    return FText::FromString(TEXT("IdealDistancePoint : From Player"));
}

FText UCPP_EQT_IdealDistancePoint::GetDescriptionDetails() const
{
    return FText::Format(
        FText::FromString(TEXT("IdealRange: {0}-{1}, EvalRange: {2}-{3}")),
        FText::AsNumber(IdealDistanceMin.DefaultValue),
        FText::AsNumber(IdealDistanceMax.DefaultValue),
        FText::AsNumber(MinEvaluationRange.DefaultValue),
        FText::AsNumber(MaxEvaluationRange.DefaultValue)
    );
}