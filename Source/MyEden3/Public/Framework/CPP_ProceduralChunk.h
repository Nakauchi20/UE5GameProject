// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/ProceduralObstacleTypes.h"
#include "CPP_ProceduralChunk.generated.h"

UCLASS()
class MYEDEN3_API ACPP_ProceduralChunk : public AActor
{
	GENERATED_BODY()

public:
	ACPP_ProceduralChunk();

	// チャンクの初期化
	void InitializeChunk(FVector2D ChunkCoord, int32 RandomSeed);

	// チャンク座標の取得
	FORCEINLINE FVector2D GetChunkCoordinate() const { return ChunkCoordinate; }

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	TArray<UStaticMeshComponent*> GetFloorTiles() const { return GeneratedMeshes; }

	// ★追加: 壁設定のゲッター
	UFUNCTION(BlueprintCallable, Category = "Chunk|Wall")
	TSubclassOf<AActor> GetWallActorClass() const { return WallActorClass; }

	UFUNCTION(BlueprintCallable, Category = "Chunk|Wall")
	int32 GetWallsPerEdge() const { return WallsPerEdge; }

	UFUNCTION(BlueprintCallable, Category = "Chunk|Wall")
	float GetWallHeight() const { return WallHeight; }

	UFUNCTION(BlueprintCallable, Category = "Chunk|Wall")
	float GetWallThickness() const { return WallThickness; }

	UFUNCTION(BlueprintCallable, Category = "Chunk|Wall")
	FName GetWallFolderName() const { return WallFolderName; }

protected:
	virtual void BeginPlay() override;

	// チャンク内にオブジェクトを生成
	void GenerateChunkContent();

	// 床の生成
	void GenerateFloor();

	// 障害物の生成
	void GenerateObstacles();

	// 重みつきランダム選択
	FObstacleCandidate* SelectObstacleByWeight(TArray<FObstacleCandidate>& Candidates);

	// スタティックメッシュ障害物の生成
	void SpawnStaticMeshObstacle(const FObstacleCandidate& Candidate, const FVector& Location, float FloorTopZ);

	// BPアクター障害物の生成
	void SpawnBPActorObstacle(const FObstacleCandidate& Candidate, const FVector& Location, float FloorTopZ);

protected:
	// チャンクのグリッド座標
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk")
	FVector2D ChunkCoordinate;

	// フォルダー名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Organization")
	FName FloorFolderName = "ProceduralFloor";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Organization")
	FName ObstacleFolderName = "ProceduralObstacles";

	// ★追加: 壁用フォルダ名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Organization")
	FName WallFolderName = "ProceduralWalls";

	// メッシュ設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Meshes")
	UStaticMesh* FloorMesh;

	// チャンクのサイズ（cm）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
	float ChunkSize = 3000.0f;

	// 正六角形の対辺間距離（横幅）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Hexagon", meta = (ClampMin = "10.0"))
	float HexagonSize = 500.0f;

	// ★追加: 壁生成設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Wall")
	TSubclassOf<AActor> WallActorClass;

	// ★追加: 1辺あたりの壁の数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Wall", meta = (ClampMin = "1", ClampMax = "10"))
	int32 WallsPerEdge = 1;

	// ★追加: 壁の高さ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Wall", meta = (ClampMin = "1.0"))
	float WallHeight = 300.0f;

	// ★追加: 壁の厚さ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Wall", meta = (ClampMin = "1.0"))
	float WallThickness = 50.0f;

	// 新しい障害物スポーンシステム
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk|Obstacles")
	TArray<FObstacleSpawnGroup> ObstacleSpawnGroups;

	// ランダムシード
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk")
	int32 Seed;

	// 生成されたメッシュコンポーネントの配列
	UPROPERTY()
	TArray<UStaticMeshComponent*> GeneratedMeshes;

	// 生成されたアクター
	UPROPERTY()
	TArray<AActor*> GeneratedActors;

	// ★追加: 生成された壁アクター
	UPROPERTY()
	TArray<AActor*> GeneratedWalls;

private:
	FRandomStream RandomStream;
};