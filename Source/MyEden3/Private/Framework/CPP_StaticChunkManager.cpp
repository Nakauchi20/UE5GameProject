// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_StaticChunkManager.h"
#include "Framework/CPP_ProceduralChunk.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ACPP_StaticChunkManager::ACPP_StaticChunkManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPP_StaticChunkManager::BeginPlay()
{
	Super::BeginPlay();

	// プレイヤーPawnを取得
	if (bGenerateAroundPlayer)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (!PlayerPawn)
		{
			UE_LOG(LogTemp, Error, TEXT("[StaticChunkManager] Player Pawn not found! Cannot generate chunks."));
			return;
		}
	}

	// 静的チャンクを生成
	GenerateStaticChunks();

	// ★追加: 全チャンク生成完了後に外周壁を生成
	GenerateOuterWalls();
}

void ACPP_StaticChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPP_StaticChunkManager::GenerateStaticChunks()
{
	if (!ChunkClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[StaticChunkManager] ChunkClass is not set!"));
		return;
	}

	// 生成位置を決定
	GenerationCenterLocation = FixedGenerationLocation;
	if (bGenerateAroundPlayer && PlayerPawn)
	{
		GenerationCenterLocation = PlayerPawn->GetActorLocation();
	}

	// プレイヤー位置からチャンク座標を取得
	CenterChunkCoord = GetChunkCoordinate(GenerationCenterLocation);

	UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] Starting generation at (%.1f, %.1f) -> Chunk(%d, %d) with %d layers"),
		GenerationCenterLocation.X, GenerationCenterLocation.Y,
		(int32)CenterChunkCoord.X, (int32)CenterChunkCoord.Y, GenerationRadius);

	// 指定層数分のチャンクを生成
	for (int32 q = -GenerationRadius; q <= GenerationRadius; ++q)
	{
		for (int32 r = -GenerationRadius; r <= GenerationRadius; ++r)
		{
			// 六角形距離の計算
			int32 HexDistance = (FMath::Abs(q) + FMath::Abs(r) + FMath::Abs(q + r)) / 2;

			if (HexDistance <= GenerationRadius)
			{
				FVector2D ChunkCoord = CenterChunkCoord + FVector2D(q, r);
				SpawnChunk(ChunkCoord);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] Generation complete - Total chunks: %d"), ActiveChunks.Num());

	// 床タイル参照を更新
	bNeedFloorTileUpdate = true;
}

void ACPP_StaticChunkManager::SpawnChunk(FVector2D ChunkCoord)
{
	if (!ChunkClass) return;

	// チャンク固有のシードを生成
	int32 ChunkSeed = WorldSeed + (int32)(ChunkCoord.X * 73856093) + (int32)(ChunkCoord.Y * 19349663);

	// チャンクのワールド座標を計算
	FVector ChunkLocation = CalculateChunkWorldLocation(ChunkCoord);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// ProceduralChunkをスポーン
	ACPP_ProceduralChunk* NewChunk = GetWorld()->SpawnActor<ACPP_ProceduralChunk>(
		ChunkClass,
		ChunkLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewChunk)
	{
		// チャンクを初期化
		NewChunk->InitializeChunk(ChunkCoord, ChunkSeed);
		ActiveChunks.Add(ChunkCoord, NewChunk);

		UE_LOG(LogTemp, Log, TEXT("[StaticChunkManager] Spawned chunk at (%d, %d) -> World(%.1f, %.1f)"),
			(int32)ChunkCoord.X, (int32)ChunkCoord.Y, ChunkLocation.X, ChunkLocation.Y);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StaticChunkManager] Failed to spawn chunk at (%d, %d)"),
			(int32)ChunkCoord.X, (int32)ChunkCoord.Y);
	}
}

FVector ACPP_StaticChunkManager::CalculateChunkWorldLocation(FVector2D ChunkCoord) const
{
	const float s = HexagonSize;
	const float h = FMath::Sqrt(3.0f) * s;

	int32 q = (int32)ChunkCoord.X;
	int32 r = (int32)ChunkCoord.Y;

	// 正確なチャンク基本ベクトル
	const FVector2D VectorQ = FVector2D(4.5f * s, -0.5f * h);
	const FVector2D VectorR = FVector2D(3.0f * s, 2.0f * h);

	FVector2D WorldPos = VectorQ * q + VectorR * r;

	return FVector(WorldPos.X, WorldPos.Y, 0.0f);
}

FVector2D ACPP_StaticChunkManager::GetChunkCoordinate(FVector WorldLocation) const
{
	const float s = HexagonSize;
	const float h = FMath::Sqrt(3.0f) * s;

	const float det = 10.5f * s * h;

	const float invA = (2.0f * h) / det;
	const float invB = (-3.0f * s) / det;
	const float invC = (0.5f * h) / det;
	const float invD = (4.5f * s) / det;

	float q = WorldLocation.X * invA + WorldLocation.Y * invB;
	float r = WorldLocation.X * invC + WorldLocation.Y * invD;

	return FVector2D(FMath::RoundToInt(q), FMath::RoundToInt(r));
}

// ★追加: 床の底面Z座標を動的に取得
float ACPP_StaticChunkManager::GetFloorBottomZ() const
{
	// 床タイルが存在する場合、最初のタイルから底面Z座標を取得
	if (AllFloorTiles.Num() > 0 && AllFloorTiles[0] && AllFloorTiles[0]->IsValidLowLevel())
	{
		UStaticMeshComponent* FloorTile = AllFloorTiles[0];

		// コンポーネントのワールド位置を取得
		FVector TileLocation = FloorTile->GetComponentLocation();

		// メッシュのバウンドとスケールから底面オフセットを計算
		if (UStaticMesh* Mesh = FloorTile->GetStaticMesh())
		{
			FBoxSphereBounds MeshBounds = Mesh->GetBounds();
			FVector TileScale = FloorTile->GetComponentScale();
			float BottomOffset = MeshBounds.BoxExtent.Z * TileScale.Z;

			// 床の底面Z座標 = タイル中心Z - 底面オフセット
			float FloorBottomZ = TileLocation.Z - BottomOffset;

			UE_LOG(LogTemp, Log, TEXT("[GetFloorBottomZ] TileZ=%.1f, Offset=%.1f, BottomZ=%.1f"),
				TileLocation.Z, BottomOffset, FloorBottomZ);

			return FloorBottomZ;
		}
	}

	// フォールバック: 床タイルがない場合は0を返す
	UE_LOG(LogTemp, Warning, TEXT("[GetFloorBottomZ] No floor tiles available, using Z=0"));
	return 0.0f;
}

// ★追加: 外周の頂点ワールド座標を取得
TArray<FVector> ACPP_StaticChunkManager::GetOuterVertexWorldLocations() const
{
	TArray<FVector> WorldVertices;

	// 床の底面Z座標を取得
	float FloorBottomZ = GetFloorBottomZ();

	// 六角形グリッドの外周6頂点（axial座標系）
	// 画像の例: 14→24→34→64→52→02→14
	TArray<FVector2D> HexVertexCoords;
	HexVertexCoords.Add(CenterChunkCoord + FVector2D(GenerationRadius, 0));           // 右 (東)
	HexVertexCoords.Add(CenterChunkCoord + FVector2D(0, GenerationRadius));           // 右上 (北東)
	HexVertexCoords.Add(CenterChunkCoord + FVector2D(-GenerationRadius, GenerationRadius)); // 左上 (北西)
	HexVertexCoords.Add(CenterChunkCoord + FVector2D(-GenerationRadius, 0));          // 左 (西)
	HexVertexCoords.Add(CenterChunkCoord + FVector2D(0, -GenerationRadius));          // 左下 (南西)
	HexVertexCoords.Add(CenterChunkCoord + FVector2D(GenerationRadius, -GenerationRadius)); // 右下 (南東)

	// チャンク座標をワールド座標に変換
	for (const FVector2D& ChunkCoord : HexVertexCoords)
	{
		FVector WorldPos = CalculateChunkWorldLocation(ChunkCoord);
		WorldPos.Z = FloorBottomZ;  // 床の底面Z座標を使用
		WorldVertices.Add(WorldPos);

		UE_LOG(LogTemp, Log, TEXT("[OuterVertex] Chunk(%.0f, %.0f) -> World(%.1f, %.1f, %.1f)"),
			ChunkCoord.X, ChunkCoord.Y, WorldPos.X, WorldPos.Y, WorldPos.Z);
	}

	return WorldVertices;
}

// ★追加: 外周壁を生成
void ACPP_StaticChunkManager::GenerateOuterWalls()
{
	// 床タイル参照を更新（床のZ座標取得に必要）
	if (bNeedFloorTileUpdate)
	{
		UpdateFloorTileReferences();
	}

	// 最初のチャンクから壁の設定を取得
	ACPP_ProceduralChunk* FirstChunk = nullptr;
	for (const auto& Pair : ActiveChunks)
	{
		FirstChunk = Pair.Value;
		break;
	}

	if (!FirstChunk)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] No chunks available for wall generation"));
		return;
	}

	// チャンクから壁の設定を取得（ゲッター関数を使用）
	TSubclassOf<AActor> WallClass = FirstChunk->GetWallActorClass();
	int32 WallsPerEdge = FirstChunk->GetWallsPerEdge();
	float WallHeight = FirstChunk->GetWallHeight();
	float WallThickness = FirstChunk->GetWallThickness();
	FName WallFolderName = FirstChunk->GetWallFolderName();

	if (!WallClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] WallActorClass is not set in ProceduralChunk"));
		return;
	}

	// 外周の頂点を取得
	TArray<FVector> WorldVertices = GetOuterVertexWorldLocations();

	if (WorldVertices.Num() != 6)
	{
		UE_LOG(LogTemp, Error, TEXT("[StaticChunkManager] Failed to get 6 outer vertices"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] Generating outer walls with %d walls per edge"), WallsPerEdge);

	// 各辺に壁を生成
	for (int32 EdgeIndex = 0; EdgeIndex < 6; ++EdgeIndex)
	{
		FVector StartVertex = WorldVertices[EdgeIndex];
		FVector EndVertex = WorldVertices[(EdgeIndex + 1) % 6];

		// 辺の長さを計算
		float EdgeLength = FVector::Dist2D(StartVertex, EndVertex);

		// 辺の方向ベクトル
		FVector EdgeDirection = (EndVertex - StartVertex).GetSafeNormal2D();

		// 壁の回転（辺に平行）
		FRotator WallRotation = EdgeDirection.Rotation();

		// 各壁のセグメントを生成
		float SegmentLength = EdgeLength / static_cast<float>(WallsPerEdge);

		for (int32 WallIndex = 0; WallIndex < WallsPerEdge; ++WallIndex)
		{
			// 壁の中心位置を計算
			// WallIndex=0: 0.5/WallsPerEdge の位置
			// WallIndex=1: 1.5/WallsPerEdge の位置
			float T = (static_cast<float>(WallIndex) + 0.5f) / static_cast<float>(WallsPerEdge);
			FVector WallCenter = FMath::Lerp(StartVertex, EndVertex, T);

			// 壁のスケールを計算
			// X: 壁の長さ（セグメント長 / 100cm ベースメッシュサイズと仮定）
			// Y: 壁の厚さ
			// Z: 壁の高さ
			FVector WallScale = FVector(
				SegmentLength / 100.0f,
				WallThickness / 100.0f,
				WallHeight / 100.0f
			);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* WallActor = GetWorld()->SpawnActor<AActor>(
				WallClass,
				WallCenter,
				WallRotation,
				SpawnParams
			);

			if (WallActor)
			{
				WallActor->SetActorScale3D(WallScale);
				SpawnedWalls.Add(WallActor);

#if WITH_EDITOR
				WallActor->SetFolderPath(WallFolderName);
#endif

				UE_LOG(LogTemp, Log, TEXT("[Wall Edge %d-%d] Center(%.1f,%.1f) Length=%.1f Scale(%.2f,%.2f,%.2f) Yaw=%.1f"),
					EdgeIndex, WallIndex,
					WallCenter.X, WallCenter.Y,
					SegmentLength,
					WallScale.X, WallScale.Y, WallScale.Z,
					WallRotation.Yaw);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] Generated %d outer walls (%d per edge x 6 edges)"),
		SpawnedWalls.Num(), WallsPerEdge);
}

void ACPP_StaticChunkManager::UpdateFloorTileReferences()
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

	UE_LOG(LogTemp, Log, TEXT("[StaticChunkManager] Updated floor tile references - Total: %d"), AllFloorTiles.Num());
}

TArray<UStaticMeshComponent*> ACPP_StaticChunkManager::GetAllFloorTiles() const
{
	// 更新が必要な場合は更新（const_castを使用）
	if (bNeedFloorTileUpdate)
	{
		const_cast<ACPP_StaticChunkManager*>(this)->UpdateFloorTileReferences();
	}

	return AllFloorTiles;
}

void ACPP_StaticChunkManager::DisappearRandomFloors(int32 Count)
{
	if (bNeedFloorTileUpdate)
	{
		UpdateFloorTileReferences();
	}

	if (AllFloorTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] No floor tiles available"));
		return;
	}

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
			FloorTile->SetVisibility(false);
			FloorTile->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			UE_LOG(LogTemp, Log, TEXT("[StaticChunkManager] Floor tile disappeared at index %d"), RandomIndex);
		}
	}
}

void ACPP_StaticChunkManager::SpawnFireOnRandomFloors(int32 Count)
{
	if (bNeedFloorTileUpdate)
	{
		UpdateFloorTileReferences();
	}

	if (AllFloorTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StaticChunkManager] No floor tiles available"));
		return;
	}

	int32 ActualCount = FMath::Min(Count, AllFloorTiles.Num());

	for (int32 i = 0; i < ActualCount; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, AllFloorTiles.Num() - 1);
		UStaticMeshComponent* FloorTile = AllFloorTiles[RandomIndex];

		if (FloorTile && FloorTile->IsValidLowLevel() && FloorTile->IsVisible())
		{
			FVector FireLocation = FloorTile->GetComponentLocation();
			FireLocation.Z += 50.0f;

			// TODO: 火のエフェクトをスポーン

			UE_LOG(LogTemp, Log, TEXT("[StaticChunkManager] Fire spawned at floor tile index %d"), RandomIndex);
		}
	}
}