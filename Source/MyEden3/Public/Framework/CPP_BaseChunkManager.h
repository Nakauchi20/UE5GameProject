// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_BaseChunkManager.generated.h"

/**
 * チャンクマネージャーの基底クラス
 * 床イベント用の共通インターフェースを提供
 */
UCLASS(Abstract)
class MYEDEN3_API ACPP_BaseChunkManager : public AActor
{
	GENERATED_BODY()

public:
	ACPP_BaseChunkManager();

	// ========== 床イベント用の仮想関数 ==========

	// ランダムな床を消滅させる
	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	virtual void DisappearRandomFloors(int32 Count);

	// ランダムな床に火を出す
	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	virtual void SpawnFireOnRandomFloors(int32 Count);

	// 全ての床タイルを取得
	UFUNCTION(BlueprintCallable, Category = "Floor Management")
	virtual TArray<UStaticMeshComponent*> GetAllFloorTiles() const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	virtual bool IsDynamicChunkManager() const { return false; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 六角形サイズ（派生クラスで使用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System|Hexagon")
	float HexagonSize = 500.0f;

	// ワールドシード
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	int32 WorldSeed = 12345;
};