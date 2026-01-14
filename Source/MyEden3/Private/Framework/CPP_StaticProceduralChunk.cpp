// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_StaticProceduralChunk.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"


ACPP_StaticProceduralChunk::ACPP_StaticProceduralChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void ACPP_StaticProceduralChunk::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_StaticProceduralChunk::InitializeStaticChunks(FVector PlayerLocation, int32 LayerCount, int32 RandomSeed)
{
	Seed = RandomSeed;
	RandomStream.Initialize(Seed);

	// プレイヤー位置からチャンク座標を取得
	FVector2D CenterChunkCoord = GetChunkCoordinate(PlayerLocation);

	UE_LOG(LogTemp, Warning, TEXT("[StaticChunk] Initializing at player location (%.1f, %.1f) -> Chunk(%d, %d) with %d layers"),
		PlayerLocation.X, PlayerLocation.Y,
		(int32)CenterChunkCoord.X, (int32)CenterChunkCoord.Y, LayerCount);

	// 指定層数分のチャンクを生成
	GenerateAllChunks(CenterChunkCoord, LayerCount);

	UE_LOG(LogTemp, Warning, TEXT("[StaticChunk] Generation complete - Total chunks: %d, Total floor tiles: %d"),
		GeneratedChunkCoords.Num(), AllGeneratedFloors.Num());

#if WITH_EDITOR
	SetFolderPath(FName("StaticProceduralChunks"));
#endif
}

void ACPP_StaticProceduralChunk::GenerateAllChunks(FVector2D CenterChunkCoord, int32 Radius)
{
	for (int32 q = -Radius; q <= Radius; ++q)
	{
		for (int32 r = -Radius; r <= Radius; ++r)
		{
			// 六角形距離の計算
			int32 HexDistance = (FMath::Abs(q) + FMath::Abs(r) + FMath::Abs(q + r)) / 2;

			if (HexDistance <= Radius)
			{
				FVector2D ChunkCoord = CenterChunkCoord + FVector2D(q, r);

				// 重複チェック
				if (GeneratedChunkCoords.Contains(ChunkCoord))
				{
					continue;
				}

				// チャンクのワールド座標を計算
				FVector ChunkWorldLocation = CalculateChunkWorldLocation(ChunkCoord);

				// 床を生成
				GenerateSingleChunkFloor(ChunkCoord, ChunkWorldLocation);

				// 障害物を生成
				GenerateSingleChunkObstacles(ChunkCoord, ChunkWorldLocation);

				// 生成済みとして記録
				GeneratedChunkCoords.Add(ChunkCoord);
			}
		}
	}
}

void ACPP_StaticProceduralChunk::GenerateSingleChunkFloor(FVector2D ChunkCoord, FVector ChunkWorldLocation)
{
	UStaticMesh* MeshToUse = FloorMesh;
	if (!MeshToUse)
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Engine/BasicShapes/Cube"));
		MeshToUse = DefaultMesh.Object;
	}
	if (!MeshToUse) return;

	FBoxSphereBounds MeshBounds = MeshToUse->GetBounds();
	FVector MeshExtent = MeshBounds.BoxExtent;

	float MeshOriginalWidth = MeshExtent.X * 2.0f;
	float MeshOriginalHeight = MeshExtent.Y * 2.0f;

	const float s = HexagonSize;
	const float w = 2.0f * s;
	const float h = FMath::Sqrt(3.0f) * s;
	const float HexHorizontalSpacing = 1.5f * s;
	const float HexVerticalSpacing = h;

	const float ScaleX = w / MeshOriginalWidth;
	const float ScaleY = h / MeshOriginalHeight;
	const float ScaleZ = ScaleX;

	// 7個の六角形配置（中心1個 + 周囲6個）
	struct FHexOffset
	{
		int32 I;
		int32 J;
		FString Description;
	};

	TArray<FHexOffset> HexPositions = {
		{ 0, 0, TEXT("Center") },
		{ 1, 0, TEXT("Right") },
		{ 1, -1, TEXT("TopRight") },
		{ 0, -1, TEXT("Top") },
		{ -1, 0, TEXT("Left") },
		{ -1, 1, TEXT("BottomLeft") },
		{ 0, 1, TEXT("Bottom") }
	};

	for (const FHexOffset& HexPos : HexPositions)
	{
		float PosX = HexHorizontalSpacing * HexPos.I;
		float PosY = HexVerticalSpacing * (HexPos.J + 0.5f * (HexPos.I % 2));

		// チャンクのワールド位置に相対位置を加算
		FVector TileLocation = ChunkWorldLocation + FVector(PosX, PosY, 0.0f);

		UStaticMeshComponent* TileMesh = NewObject<UStaticMeshComponent>(this);
		TileMesh->SetupAttachment(RootComponent);
		TileMesh->SetStaticMesh(MeshToUse);
		TileMesh->RegisterComponent();

		TileMesh->SetWorldLocation(TileLocation);
		TileMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, ScaleZ));
		TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TileMesh->SetCollisionObjectType(ECC_WorldStatic);

		AllGeneratedFloors.Add(TileMesh);
	}
}

void ACPP_StaticProceduralChunk::GenerateSingleChunkObstacles(FVector2D ChunkCoord, FVector ChunkWorldLocation)
{
	if (ObstacleSpawnGroups.Num() == 0) return;

	// チャンク固有のシードを生成
	int32 ChunkSeed = Seed + (int32)(ChunkCoord.X * 73856093) + (int32)(ChunkCoord.Y * 19349663);
	FRandomStream ChunkRandomStream;
	ChunkRandomStream.Initialize(ChunkSeed);

	float HalfChunkSize = 1500.0f; // 適切なサイズに調整

	for (FObstacleSpawnGroup& Group : ObstacleSpawnGroups)
	{
		if (ChunkRandomStream.FRand() > Group.SpawnChance)
		{
			continue;
		}

		if (Group.Candidates.Num() == 0) continue;

		int32 SpawnCount = ChunkRandomStream.RandRange(Group.MinCount, Group.MaxCount);

		for (int32 i = 0; i < SpawnCount; ++i)
		{
			FObstacleCandidate* SelectedCandidate = SelectObstacleByWeight(Group.Candidates);
			if (!SelectedCandidate) continue;

			// チャンク内のランダム位置
			FVector LocalSpawnLocation = FVector(
				ChunkRandomStream.FRandRange(-HalfChunkSize + 200.0f, HalfChunkSize - 200.0f),
				ChunkRandomStream.FRandRange(-HalfChunkSize + 200.0f, HalfChunkSize - 200.0f),
				0.0f
			);

			FVector WorldSpawnLocation = ChunkWorldLocation + LocalSpawnLocation;

			if (SelectedCandidate->ObstacleType == EObstacleType::StaticMesh)
			{
				SpawnStaticMeshObstacle(*SelectedCandidate, WorldSpawnLocation, ChunkCoord);
			}
			else if (SelectedCandidate->ObstacleType == EObstacleType::BPActor)
			{
				SpawnBPActorObstacle(*SelectedCandidate, WorldSpawnLocation, ChunkCoord);
			}
		}
	}
}

FVector ACPP_StaticProceduralChunk::CalculateChunkWorldLocation(FVector2D ChunkCoord) const
{
	const float s = HexagonSize;
	const float h = FMath::Sqrt(3.0f) * s;

	int32 q = (int32)ChunkCoord.X;
	int32 r = (int32)ChunkCoord.Y;

	const FVector2D VectorQ = FVector2D(4.5f * s, -0.5f * h);
	const FVector2D VectorR = FVector2D(3.0f * s, 2.0f * h);

	FVector2D WorldPos = VectorQ * q + VectorR * r;

	return FVector(WorldPos.X, WorldPos.Y, 0.0f);
}

FVector2D ACPP_StaticProceduralChunk::GetChunkCoordinate(FVector WorldLocation) const
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

FObstacleCandidate* ACPP_StaticProceduralChunk::SelectObstacleByWeight(TArray<FObstacleCandidate>& Candidates)
{
	if (Candidates.Num() == 0) return nullptr;

	float TotalWeight = 0.0f;
	for (const FObstacleCandidate& Candidate : Candidates)
	{
		TotalWeight += Candidate.Weight;
	}

	if (TotalWeight <= 0.0f) return nullptr;

	float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);

	float CurrentWeight = 0.0f;
	for (FObstacleCandidate& Candidate : Candidates)
	{
		CurrentWeight += Candidate.Weight;
		if (RandomValue <= CurrentWeight)
		{
			return &Candidate;
		}
	}

	return &Candidates[0];
}

void ACPP_StaticProceduralChunk::SpawnStaticMeshObstacle(const FObstacleCandidate& Candidate, const FVector& Location, FVector2D ChunkCoord)
{
	UStaticMesh* MeshToUse = Candidate.StaticMesh;
	if (!MeshToUse)
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Engine/BasicShapes/Cube"));
		MeshToUse = DefaultMesh.Object;
	}
	if (!MeshToUse) return;

	UStaticMeshComponent* ObstacleMeshComp = NewObject<UStaticMeshComponent>(this);
	ObstacleMeshComp->SetupAttachment(RootComponent);
	ObstacleMeshComp->SetStaticMesh(MeshToUse);
	ObstacleMeshComp->RegisterComponent();

	FVector ObstacleSize = FVector(
		RandomStream.FRandRange(Candidate.MinSize.X, Candidate.MaxSize.X),
		RandomStream.FRandRange(Candidate.MinSize.Y, Candidate.MaxSize.Y),
		RandomStream.FRandRange(Candidate.MinSize.Z, Candidate.MaxSize.Z)
	);

	FVector AdjustedLocation = Location;
	AdjustedLocation.Z = ObstacleSize.Z * 0.5f;

	ObstacleMeshComp->SetWorldLocation(AdjustedLocation);
	ObstacleMeshComp->SetRelativeScale3D(ObstacleSize / 100.0f);
	ObstacleMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ObstacleMeshComp->SetCollisionObjectType(ECC_WorldStatic);

}

void ACPP_StaticProceduralChunk::SpawnBPActorObstacle(const FObstacleCandidate& Candidate, const FVector& Location, FVector2D ChunkCoord)
{
	if (!Candidate.ActorClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		Candidate.ActorClass,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (SpawnedActor)
	{
		AllGeneratedActors.Add(SpawnedActor);

#if WITH_EDITOR
		SpawnedActor->SetFolderPath(ObstacleFolderName);
#endif
	}
}

