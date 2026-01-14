// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CPP_LevelConfigDataAsset.generated.h"

class ACPP_BaseChunkManager;
class UCPP_BaseLevelSubsystem;

/**
 * レベル設定用DataAsset
 * チャンクマネージャーとゲーム進行Subsystemの組み合わせを定義
 */
UCLASS(BlueprintType, Blueprintable)
class MYEDEN3_API UCPP_LevelConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// ========== レベル情報 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Info")
	FText LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Info", meta = (MultiLine = true))
	FText LevelDescription;

	// PrimaryDataAsset用の識別子
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("LevelConfig", GetFName());
	}

	// ========== Subsystem設定 ==========

	// ★ 変更: 単一 → 配列、型も変更
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Systems", meta = (AllowAbstract = "false"))
	TArray<TSubclassOf<UCPP_BaseLevelSubsystem>> SubsystemClasses;

	// ========== チャンクマネージャー設定 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager")
	TSubclassOf<ACPP_BaseChunkManager> ChunkManagerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager")
	int32 WorldSeed = 12345;

	// ========== 動的チャンク専用設定 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager|Dynamic")
	int32 ChunkLoadRadius = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager|Dynamic")
	int32 ChunkUnloadRadius = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager|Dynamic")
	float UpdateInterval = 0.5f;

	// ========== 静的チャンク専用設定 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager|Static")
	int32 StaticGenerationRadius = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager|Static")
	bool bGenerateAroundPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Manager|Static")
	FVector FixedGenerationLocation = FVector::ZeroVector;

	

	
};