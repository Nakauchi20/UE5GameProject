// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/ProceduralObstacleTypes.h"
#include "CPP_StaticProceduralChunk.generated.h"

UCLASS()
class MYEDEN3_API ACPP_StaticProceduralChunk : public AActor
{
	GENERATED_BODY()
	
public:
	ACPP_StaticProceduralChunk();

	// 静的チャンク群の初期化（プレイヤー位置と層数を指定）
	void InitializeStaticChunks(FVector PlayerLocation, int32 LayerCount, int32 RandomSeed);

	// 全ての床タイルを取得
	UFUNCTION(BlueprintCallable, Category = "Static Chunk")
	TArray<UStaticMeshComponent*> GetAllFloorTiles() const { return AllGeneratedFloors; }

protected:
	virtual void BeginPlay() override;

	// 指定層数分のチャンクを生成
	void GenerateAllChunks(FVector2D CenterChunkCoord, int32 Radius);

	// 単一チャンクの床を生成
	void GenerateSingleChunkFloor(FVector2D ChunkCoord, FVector ChunkWorldLocation);

	// 単一チャンクの障害物を生成
	void GenerateSingleChunkObstacles(FVector2D ChunkCoord, FVector ChunkWorldLocation);

	// チャンク座標からワールド座標を計算
	FVector CalculateChunkWorldLocation(FVector2D ChunkCoord) const;

	// プレイヤー位置からチャンク座標を取得
	FVector2D GetChunkCoordinate(FVector WorldLocation) const;

	// 障害物選択（重み付き）
	FObstacleCandidate* SelectObstacleByWeight(TArray<FObstacleCandidate>& Candidates);

	// 障害物生成
	void SpawnStaticMeshObstacle(const FObstacleCandidate& Candidate, const FVector& Location, FVector2D ChunkCoord);
	void SpawnBPActorObstacle(const FObstacleCandidate& Candidate, const FVector& Location, FVector2D ChunkCoord);

protected:
	// フォルダー名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk|Organization")
	FName FloorFolderName = "StaticProceduralFloor";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk|Organization")
	FName ObstacleFolderName = "StaticProceduralObstacles";

	// メッシュ設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk|Meshes")
	UStaticMesh* FloorMesh;

	// 六角形サイズ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk|Hexagon", meta = (ClampMin = "10.0"))
	float HexagonSize = 500.0f;

	// 障害物スポーン設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Chunk|Obstacles")
	TArray<FObstacleSpawnGroup> ObstacleSpawnGroups;

	// ランダムシード
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Static Chunk")
	int32 Seed;

private:
	// 全ての生成された床メッシュ
	UPROPERTY()
	TArray<UStaticMeshComponent*> AllGeneratedFloors;

	// 全ての生成されたアクター
	UPROPERTY()
	TArray<AActor*> AllGeneratedActors;

	// ランダムストリーム
	FRandomStream RandomStream;

	// 生成済みチャンク座標の記録
	TSet<FVector2D> GeneratedChunkCoords;
};
