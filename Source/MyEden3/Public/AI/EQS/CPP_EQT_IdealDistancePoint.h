// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/EQS/CPP_EQT_Base.h"
#include "DataProviders/AIDataProvider.h"
#include "CPP_EQT_IdealDistancePoint.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class MYEDEN3_API UCPP_EQT_IdealDistancePoint : public UCPP_EQT_Base
{
	GENERATED_BODY()

public:
	UCPP_EQT_IdealDistancePoint(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

public:

	/** ï]âøäJénãóó£(â∫å¿) */
	UPROPERTY(EditAnywhere, Category = "EQS",
		meta = (ClampMin = "0.0"))
	FAIDataProviderFloatValue MinEvaluationRange;
	/** ãóó£Ç™Ç±ÇÃílÇ…ãﬂÇ¢ÇŸÇ«ï]âøÇ™çÇÇ≠Ç»ÇÈ */
	UPROPERTY(EditAnywhere, Category = "EQS",
		meta = (ClampMin = "0.0"))
	FAIDataProviderFloatValue IdealDistanceMin;

	UPROPERTY(EditAnywhere, Category = "EQS",
		meta = (ClampMin = "0.0"))
	FAIDataProviderFloatValue IdealDistanceMax;
	/** ï]âøèIóπãóó£(è„å¿) */
	UPROPERTY(EditAnywhere, Category = "EQS",
		meta = (ClampMin = "0.0"))
	FAIDataProviderFloatValue MaxEvaluationRange;
};