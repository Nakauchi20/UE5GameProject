// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_ChunkManager.h"
#include "Framework/CPP_ProceduralChunk.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ACPP_ChunkManager::ACPP_ChunkManager()
{
	PrimaryActorTick.bCanEverTick = true;
	LastPlayerChunkCoord = FVector2D(-999999, -999999);
}

void ACPP_ChunkManager::BeginPlay()
{
	Super::BeginPlay();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PlayerPawn)
	{
		FVector PlayerLocation = PlayerPawn->GetActorLocation();
		UpdateChunks(PlayerLocation);
	}
}

void ACPP_ChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerPawn) return;

	TimeSinceLastUpdate += DeltaTime;

	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		TimeSinceLastUpdate = 0.0f;
		UpdateChunks(PlayerPawn->GetActorLocation());
	}
}

void ACPP_ChunkManager::UpdateChunks(FVector PlayerLocation)
{
	FVector2D PlayerChunkCoord = GetChunkCoordinate(PlayerLocation);

	// ★ デバッグ用ログ追加
	UE_LOG(LogTemp, Warning, TEXT("[ChunkManager] Player at World(%.1f, %.1f) -> Chunk(%d, %d)"),
		PlayerLocation.X, PlayerLocation.Y,
		(int32)PlayerChunkCoord.X, (int32)PlayerChunkCoord.Y);

	if (PlayerChunkCoord != LastPlayerChunkCoord)
	{
		GenerateChunksAroundPlayer(PlayerChunkCoord);
		RemoveDistantChunks(PlayerChunkCoord);
		LastPlayerChunkCoord = PlayerChunkCoord;

		// チャンクが変更されたので床参照を更新
		bNeedFloorTileUpdate = true;
	}
}

void ACPP_ChunkManager::GenerateChunksAroundPlayer(FVector2D PlayerChunkCoord)
{
	for (int32 q = -ChunkLoadRadius; q <= ChunkLoadRadius; ++q)
	{
		for (int32 r = -ChunkLoadRadius; r <= ChunkLoadRadius; ++r)
		{
			// 六角形距離の計算
			int32 HexDistance = (FMath::Abs(q) + FMath::Abs(r) + FMath::Abs(q + r)) / 2;

			if (HexDistance <= ChunkLoadRadius)
			{
				FVector2D ChunkCoord = PlayerChunkCoord + FVector2D(q, r);

				if (!IsChunkGenerated(ChunkCoord))
				{
					SpawnChunk(ChunkCoord);
				}
			}
		}
	}
}

void ACPP_ChunkManager::RemoveDistantChunks(FVector2D PlayerChunkCoord)
{
	TArray<FVector2D> ChunksToRemove;

	for (auto& Pair : ActiveChunks)
	{
		FVector2D ChunkCoord = Pair.Key;

		int32 dq = (int32)(ChunkCoord.X - PlayerChunkCoord.X);
		int32 dr = (int32)(ChunkCoord.Y - PlayerChunkCoord.Y);
		int32 HexDistance = (FMath::Abs(dq) + FMath::Abs(dr) + FMath::Abs(dq + dr)) / 2;

		if (HexDistance > ChunkUnloadRadius)
		{
			ChunksToRemove.Add(ChunkCoord);
		}
	}

	for (const FVector2D& ChunkCoord : ChunksToRemove)
	{
		if (ACPP_ProceduralChunk* Chunk = ActiveChunks.FindRef(ChunkCoord))
		{
			Chunk->Destroy();
			ActiveChunks.Remove(ChunkCoord);
			UE_LOG(LogTemp, Warning, TEXT("[ChunkManager] Unloaded chunk at (%d, %d)"),
				(int32)ChunkCoord.X, (int32)ChunkCoord.Y);
		}
	}
}

FVector2D ACPP_ChunkManager::GetChunkCoordinate(FVector WorldLocation) const
{
	if (bUseHexagonalChunks)
	{
		const float s = HexagonSize;                    // 500
		const float h = FMath::Sqrt(3.0f) * s;          // 866.025

		const float det = 10.5f * s * h;  // 10.5 * 500 * 866.025 = 4,546,631.25

		// 逆行列の各要素
		const float invA = (2.0f * h) / det;     //  2h / det
		const float invB = (-3.0f * s) / det;    // -3s / det  
		const float invC = (0.5f * h) / det;     //  0.5h / det 
		const float invD = (4.5f * s) / det;     //  4.5s / det

		// q = invA * X + invB * Y
		// r = invC * X + invD * Y
		float q = WorldLocation.X * invA + WorldLocation.Y * invB;
		float r = WorldLocation.X * invC + WorldLocation.Y * invD;

		return FVector2D(FMath::RoundToInt(q), FMath::RoundToInt(r));
	}
	else
	{
		return FVector2D(
			FMath::FloorToInt(WorldLocation.X / ChunkSize),
			FMath::FloorToInt(WorldLocation.Y / ChunkSize)
		);
	}
}

bool ACPP_ChunkManager::IsChunkGenerated(FVector2D ChunkCoord) const
{
	return ActiveChunks.Contains(ChunkCoord);
}


void ACPP_ChunkManager::SpawnChunk(FVector2D ChunkCoord)
{
	if (!ChunkClass) return;

	int32 ChunkSeed = WorldSeed + (int32)(ChunkCoord.X * 73856093) + (int32)(ChunkCoord.Y * 19349663);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	FVector ChunkLocation = FVector::ZeroVector;

	if (bUseHexagonalChunks)
	{

		const float s = HexagonSize;                    // 500
		const float h = FMath::Sqrt(3.0f) * s;          // 866.025

		int32 q = (int32)ChunkCoord.X;
		int32 r = (int32)ChunkCoord.Y;

		// 正確なチャンク基本ベクトル（元のログから逆算）
		const FVector2D VectorQ = FVector2D(4.5f * s, -0.5f * h);   // (2250, -433.0125)
		const FVector2D VectorR = FVector2D(3.0f * s, 2.0f * h);    // (1500, 1732.05)

		// ワールド座標を計算（×2は不要、ベクトル自体が正しい間隔）
		FVector2D WorldPos = VectorQ * q + VectorR * r;

		ChunkLocation = FVector(WorldPos.X, WorldPos.Y, 0.0f);

		UE_LOG(LogTemp, Warning, TEXT("[ChunkManager] Coord(%d, %d) -> World(%.1f, %.1f)"),
			q, r, WorldPos.X, WorldPos.Y);
	}
	else
	{
		ChunkLocation = FVector(
			ChunkCoord.X * ChunkSize,
			ChunkCoord.Y * ChunkSize,
			0.0f
		);
	}

	ACPP_ProceduralChunk* NewChunk = GetWorld()->SpawnActor<ACPP_ProceduralChunk>(
		ChunkClass,
		ChunkLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewChunk)
	{
		NewChunk->InitializeChunk(ChunkCoord, ChunkSeed);
		ActiveChunks.Add(ChunkCoord, NewChunk);
		UE_LOG(LogTemp, Warning, TEXT("[ChunkManager] Successfully spawned chunk at (%d, %d)"),
			(int32)ChunkCoord.X, (int32)ChunkCoord.Y);
	}
}



// ========== Floor Event Commands ==========

void ACPP_ChunkManager::DisappearRandomFloors(int32 Count)
{
	// 床参照が更新必要ならば更新
	if (bNeedFloorTileUpdate)
	{
		UpdateFloorTileReferences();
	}

	if (AllFloorTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChunkManager] No floor tiles available for disappear event"));
		return;
	}

	// ランダムに床を選択して消す
	int32 ActualCount = FMath::Min(Count, AllFloorTiles.Num());
	TArray<int32> SelectedIndices;

	for (int32 i = 0; i < ActualCount; ++i)
	{
		int32 RandomIndex;
		do
		{
			RandomIndex = FMath::RandRange(0, AllFloorTiles.Num() - 1);
		} while (SelectedIndices.Contains(RandomIndex));

		SelectedIndices.Add(RandomIndex);

		UStaticMeshComponent* FloorTile = AllFloorTiles[RandomIndex];
		if (FloorTile && FloorTile->IsValidLowLevel())
		{
			// 床を非表示にする
			FloorTile->SetVisibility(false);
			FloorTile->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			UE_LOG(LogTemp, Log, TEXT("[ChunkManager] Floor tile disappeared at index %d"), RandomIndex);
		}
	}
}

TArray<UStaticMeshComponent*> ACPP_ChunkManager::GetAllFloorTiles() const
{
	return AllFloorTiles;
}

void ACPP_ChunkManager::UpdateFloorTileReferences()
{
	AllFloorTiles.Empty();

	for (const auto& Pair : ActiveChunks)
	{
		ACPP_ProceduralChunk* Chunk = Pair.Value;
		if (Chunk)
		{
			TArray<UStaticMeshComponent*> ChunkFloors = Chunk->GetFloorTiles();
			AllFloorTiles.Append(ChunkFloors);
		}
	}

	bNeedFloorTileUpdate = false;

	UE_LOG(LogTemp, Log, TEXT("[ChunkManager] Updated floor tile references - Total: %d"), AllFloorTiles.Num());
}