// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/CPP_BaseChunkManager.h"
#include "CPP_ProceduralChunk.h"
#include "CPP_ChunkManager.generated.h"


UCLASS()
class MYEDEN3_API ACPP_ChunkManager : public ACPP_BaseChunkManager
{
	GENERATED_BODY()

public:
	ACPP_ChunkManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void UpdateChunks(FVector PlayerLocation);

	// ========== Floor Event Commands ==========
	virtual void DisappearRandomFloors(int32 Count) override;
	virtual TArray<UStaticMeshComponent*> GetAllFloorTiles() const override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	int32 ChunkLoadRadius = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	int32 ChunkUnloadRadius = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	float UpdateInterval = 0.5f;

	virtual bool IsDynamicChunkManager() const override { return true; }

protected:
	FVector CalculateHexClusterPosition(FVector2D ChunkCoord) const;
	void GenerateChunksAroundPlayer(FVector2D PlayerChunkCoord);
	void RemoveDistantChunks(FVector2D PlayerChunkCoord);
	FVector2D GetChunkCoordinate(FVector WorldLocation) const;
	bool IsChunkGenerated(FVector2D ChunkCoord) const;
	void SpawnChunk(FVector2D ChunkCoord);

	// 床タイルの参照を更新
	void UpdateFloorTileReferences();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	TSubclassOf<ACPP_ProceduralChunk> ChunkClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	float ChunkSize = 3000.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk System")
	bool bUseHexagonalChunks = true;


private:
	UPROPERTY()
	TMap<FVector2D, ACPP_ProceduralChunk*> ActiveChunks;

	// 全ての床タイルへの参照キャッシュ
	UPROPERTY()
	TArray<UStaticMeshComponent*> AllFloorTiles;

	FVector2D LastPlayerChunkCoord;
	float TimeSinceLastUpdate = 0.0f;

	UPROPERTY()
	APawn* PlayerPawn;

	// 床参照の更新が必要かフラグ
	bool bNeedFloorTileUpdate = true;
};