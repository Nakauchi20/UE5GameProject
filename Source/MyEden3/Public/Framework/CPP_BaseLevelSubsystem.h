// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CPP_BaseLevelSubsystem.generated.h"

class ACPP_BaseChunkManager;

/**
 * レベルSubsystemの基底クラス
 * 共通の初期化インターフェースを提供
 */
UCLASS(Abstract)
class MYEDEN3_API UCPP_BaseLevelSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 共通の初期化関数
	UFUNCTION(BlueprintCallable, Category = "Level Subsystem")
	virtual void InitializeLevelSubsystem(ACPP_BaseChunkManager* InChunkManager);

	// ChunkManagerの取得
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Level Subsystem")
	ACPP_BaseChunkManager* GetChunkManager() const { return ChunkManager; }

protected:
	UPROPERTY()
	ACPP_BaseChunkManager* ChunkManager;
};