// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPP_PC_Base.generated.h"

/**
 * 
 */
UCLASS()
class MYEDEN3_API ACPP_PC_Base : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

public:
	ACPP_PC_Base();

protected:
	virtual void BeginPlay();
};
