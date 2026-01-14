// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPP_GM_Base.generated.h"

class UCPP_LevelConfigDataAsset;
class ACPP_BaseChunkManager;

/**
 * 基本的なゲームモードクラス
 * レベル設定に基づいてチャンクマネージャーとSubsystemを初期化
 */
UCLASS()
class MYEDEN3_API ACPP_GM_Base : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACPP_GM_Base();

protected:
	virtual void BeginPlay() override;

	// レベル設定を読み込んで初期化
	void InitializeLevelFromConfig();

	// チャンクマネージャーをスポーン
	ACPP_BaseChunkManager* SpawnChunkManagerFromConfig(UCPP_LevelConfigDataAsset* Config);

	void InitializeSubsystems(UCPP_LevelConfigDataAsset* Config, ACPP_BaseChunkManager* ChunkManager);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Configuration")
	UCPP_LevelConfigDataAsset* LevelConfig;

private:
	UPROPERTY()
	ACPP_BaseChunkManager* ChunkManager;
};