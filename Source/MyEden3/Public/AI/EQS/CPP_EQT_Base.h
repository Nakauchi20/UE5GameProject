// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "DataProviders/AIDataProvider.h"
#include "CPP_EQT_Base.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MYEDEN3_API UCPP_EQT_Base : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UCPP_EQT_Base(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	TSubclassOf<UEnvQueryContext> SightFrom;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
};
