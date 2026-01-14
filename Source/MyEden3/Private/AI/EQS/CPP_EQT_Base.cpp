// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/CPP_EQT_Base.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

UCPP_EQT_Base::UCPP_EQT_Base(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();//actorを対象とする場合はこれを指定 #include忘れずに
	SightFrom = UEnvQueryContext_Querier::StaticClass();// 必要であればContextの初期化
}

void UCPP_EQT_Base::RunTest(FEnvQueryInstance& QueryInstance) const
{
	// Ownerは存在するか
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	// Editor側で設定したMAX値の取得
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	// Editor側で設定したMIN値の取得
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();

	TArray<AActor*> ContextActors;
	if (!QueryInstance.PrepareContext(SightFrom, ContextActors))
	{
		return;
	}

	float score = 0.0f;
	// テスト対象のアイテムをここでスコアリングする
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		// scoreを確定する処理をここに…

		// フィルタリングかスコア設定だけか又はその両方か、
		// 上限値、下限値だけを見るか又はその範囲内か
		// 上限値と下限値を引数として渡す
		It.SetScore(TestPurpose, FilterType, score, MinThresholdValue, MaxThresholdValue);
	}
}


FText UCPP_EQT_Base::GetDescriptionTitle() const
{
	return FText::FromString(FString::Printf(TEXT("%s: From %s"),
		*Super::GetDescriptionTitle().ToString(),
		*UEnvQueryTypes::DescribeContext(SightFrom).ToString()));
}

FText UCPP_EQT_Base::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}