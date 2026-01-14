// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "CPP_AssetManager.generated.h"

UCLASS()
class MYEDEN3_API UCPP_AssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UCPP_AssetManager& Get();

};