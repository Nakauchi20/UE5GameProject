// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Framework/CPP_BaseChunkManager.h"
#include "CPP_StaticChunkManager.generated.h"

class ACPP_ProceduralChunk;

UCLASS()
class MYEDEN3_API ACPP_StaticChunkManager : public ACPP_BaseChunkManager
{
	GENERATED_BODY()

public:
	ACPP_StaticChunkManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== 基底クラスのオーバーライド ==========
	virtual void DisappearRandomFloors(int32 Count) override;
	virtual void SpawnFireOnRandomFloors(int32 Count) override;
	virtual TArray<UStaticMeshComponent*> GetAllFloorTiles() const override;

	// ========== Configuration Properties ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk System", meta = (ClampMin = "1", ClampMax = "10"))
	int32 GenerationRadius = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk System")
	bool bGenerateAroundPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk System", meta = (EditCondition = "!bGenerateAroundPlayer"))
	FVector FixedGenerationLocation = FVector::ZeroVector;

	// ========== Public Helper Functions ==========
	UFUNCTION(BlueprintCallable, Category = "Static Chunk System")
	FVector CalculateChunkWorldLocation(FVector2D ChunkCoord) const;

	UFUNCTION(BlueprintCallable, Category = "Static Chunk System")
	FVector2D GetChunkCoordinate(FVector WorldLocation) const;

	// ★追加: 外周壁を生成（全チャンク生成完了後に呼び出す）
	UFUNCTION(BlueprintCallable, Category = "Static Chunk System|Wall")
	void GenerateOuterWalls();

protected:
	// 静的チャンク群を生成
	void GenerateStaticChunks();

	// 単一チャンクをスポーン
	void SpawnChunk(FVector2D ChunkCoord);

	// 床タイル参照を更新
	void UpdateFloorTileReferences();

	// ★追加: 外周の頂点座標を取得
	TArray<FVector> GetOuterVertexWorldLocations() const;

	// ★追加: 床の底面Z座標を動的に取得
	float GetFloorBottomZ() const;

	// 使用するチャンククラス（ProceduralChunkを使用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk System")
	TSubclassOf<ACPP_ProceduralChunk> ChunkClass;

private:
	// 生成されたチャンクのマップ
	UPROPERTY()
	TMap<FVector2D, ACPP_ProceduralChunk*> ActiveChunks;

	// 全ての床タイルへの参照キャッシュ
	UPROPERTY()
	TArray<UStaticMeshComponent*> AllFloorTiles;

	// ★追加: 生成された壁アクター
	UPROPERTY()
	TArray<AActor*> SpawnedWalls;

	// プレイヤーPawn
	UPROPERTY()
	APawn* PlayerPawn;

	// 床参照の更新が必要かフラグ
	bool bNeedFloorTileUpdate = true;

	// ★追加: 中心チャンク座標（壁生成時に使用）
	FVector2D CenterChunkCoord;

	// ★追加: 生成時の中心位置
	FVector GenerationCenterLocation;
};