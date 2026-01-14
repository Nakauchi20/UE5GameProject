// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_ProceduralChunk.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

ACPP_ProceduralChunk::ACPP_ProceduralChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	// ルートコンポーネント設定
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void ACPP_ProceduralChunk::BeginPlay()
{
	Super::BeginPlay();

	// ★床チャンクアクター自体をフォルダに配置
#if WITH_EDITOR
	SetFolderPath(FloorFolderName);
#endif
}

void ACPP_ProceduralChunk::InitializeChunk(FVector2D ChunkCoord, int32 RandomSeed)
{
	ChunkCoordinate = ChunkCoord;
	Seed = RandomSeed;
	RandomStream.Initialize(Seed);

	// コンテンツ生成
	GenerateChunkContent();
}

void ACPP_ProceduralChunk::GenerateChunkContent()
{
	GenerateFloor();
	GenerateObstacles();
}

void ACPP_ProceduralChunk::GenerateFloor()
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

	int32 TileCount = 0;

	// 7個の六角形配置（中心1個 + 周囲6個）
	struct FHexOffset
	{
		int32 I;
		int32 J;
		FString Description;
	};

	// フラットトップ六角形の周囲6方向のオフセット
	TArray<FHexOffset> HexPositions = {
		{ 0, 0, TEXT("Center") },
		{ 1, 0, TEXT("Right") },
		{ 1, -1, TEXT("TopRight") },
		{ 0, -1, TEXT("Top") },
		{ -1, 0, TEXT("Left") },
		{ -1, 1, TEXT("BottomLeft") },
		{ 0, 1, TEXT("Bottom") }
	};

	float ChunkCenterX = 0.0f;
	float ChunkCenterY = 0.0f;

	for (const FHexOffset& HexPos : HexPositions)
	{
		float PosX = ChunkCenterX + HexHorizontalSpacing * HexPos.I;
		float PosY = ChunkCenterY + HexVerticalSpacing * (HexPos.J + 0.5f * (HexPos.I % 2));

		FVector TileLocation = FVector(PosX, PosY, 0.0f);

		UStaticMeshComponent* TileMesh = NewObject<UStaticMeshComponent>(this);
		TileMesh->SetupAttachment(RootComponent);
		TileMesh->SetStaticMesh(MeshToUse);
		TileMesh->RegisterComponent();

		TileMesh->SetRelativeLocation(TileLocation);
		FVector Scale = FVector(ScaleX, ScaleY, ScaleZ);
		TileMesh->SetRelativeScale3D(Scale);

		TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TileMesh->SetCollisionObjectType(ECC_WorldStatic);

		// ★床タイルのみを配列に追加
		GeneratedMeshes.Add(TileMesh);
		TileCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("[GenerateFloor] Chunk (%.1f, %.1f): Generated %d floor tiles"),
		ChunkCoordinate.X, ChunkCoordinate.Y, TileCount);
}

void ACPP_ProceduralChunk::GenerateObstacles()
{
	if (ObstacleSpawnGroups.Num() == 0)
	{
		return;
	}

	if (GeneratedMeshes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No floor tiles generated for Chunk (%f, %f)"), ChunkCoordinate.X, ChunkCoordinate.Y);
		return;
	}

	// 床の上面の高さを計算
	float FloorTopZ = 0.0f;
	if (GeneratedMeshes[0] && GeneratedMeshes[0]->GetStaticMesh())
	{
		FBoxSphereBounds FloorBounds = GeneratedMeshes[0]->GetStaticMesh()->GetBounds();
		FVector FloorScale = GeneratedMeshes[0]->GetRelativeScale3D();
		FloorTopZ = FloorBounds.BoxExtent.Z * FloorScale.Z;
	}

	// 各グループを処理
	for (FObstacleSpawnGroup& Group : ObstacleSpawnGroups)
	{
		if (RandomStream.FRand() > Group.SpawnChance)
		{
			continue;
		}

		if (Group.Candidates.Num() == 0)
		{
			continue;
		}

		int32 SpawnCount = RandomStream.RandRange(Group.MinCount, Group.MaxCount);

		for (int32 i = 0; i < SpawnCount; ++i)
		{
			FObstacleCandidate* SelectedCandidate = SelectObstacleByWeight(Group.Candidates);
			if (!SelectedCandidate)
			{
				continue;
			}

			// ランダムな床タイルを選択
			int32 RandomTileIndex = RandomStream.RandRange(0, GeneratedMeshes.Num() - 1);
			UStaticMeshComponent* SelectedTile = GeneratedMeshes[RandomTileIndex];

			if (!SelectedTile || !SelectedTile->IsValidLowLevel())
			{
				continue;
			}

			// 床タイルの位置を取得
			FVector TileLocation = SelectedTile->GetRelativeLocation();
			FBoxSphereBounds TileBounds = SelectedTile->GetStaticMesh()->GetBounds();
			float TileRadius = TileBounds.SphereRadius * SelectedTile->GetRelativeScale3D().X;

			// 床タイルの範囲内でランダムオフセット
			float RandomRadius = RandomStream.FRandRange(0.0f, TileRadius * 0.6f);
			float RandomAngle = RandomStream.FRandRange(0.0f, 2.0f * PI);

			FVector RandomOffset = FVector(
				RandomRadius * FMath::Cos(RandomAngle),
				RandomRadius * FMath::Sin(RandomAngle),
				0.0f
			);

			FVector SpawnLocation = TileLocation + RandomOffset;
			SpawnLocation.Z = FloorTopZ;

			// 種類に応じて生成
			if (SelectedCandidate->ObstacleType == EObstacleType::StaticMesh)
			{
				SpawnStaticMeshObstacle(*SelectedCandidate, SpawnLocation, FloorTopZ);
			}
			else if (SelectedCandidate->ObstacleType == EObstacleType::BPActor)
			{
				SpawnBPActorObstacle(*SelectedCandidate, SpawnLocation, FloorTopZ);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[GenerateObstacles] Chunk (%.1f, %.1f): Spawned obstacles from %d groups"),
		ChunkCoordinate.X, ChunkCoordinate.Y, ObstacleSpawnGroups.Num());
}

FObstacleCandidate* ACPP_ProceduralChunk::SelectObstacleByWeight(TArray<FObstacleCandidate>& Candidates)
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

void ACPP_ProceduralChunk::SpawnStaticMeshObstacle(const FObstacleCandidate& Candidate, const FVector& Location, float FloorTopZ)
{
	UStaticMesh* MeshToUse = Candidate.StaticMesh;
	if (!MeshToUse)
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Engine/BasicShapes/Cube"));
		MeshToUse = DefaultMesh.Object;
	}
	if (!MeshToUse) return;

	// ランダムサイズを計算
	FVector ObstacleSize = FVector(
		RandomStream.FRandRange(Candidate.MinSize.X, Candidate.MaxSize.X),
		RandomStream.FRandRange(Candidate.MinSize.Y, Candidate.MaxSize.Y),
		RandomStream.FRandRange(Candidate.MinSize.Z, Candidate.MaxSize.Z)
	);
	FVector ObstacleScale = ObstacleSize / 100.0f;

	// ワールド座標を計算
	FVector WorldLocation = GetActorLocation() + Location;

	// Z座標の計算
	if (Candidate.bGrounded)
	{
		// 床に接地: Pivotが底面中央なので、床の上面に直接配置
		WorldLocation.Z = GetActorLocation().Z + FloorTopZ;
	}
	else
	{
		// 空中に配置: 床の上面からランダムな高さに配置
		float RandomHeight = RandomStream.FRandRange(Candidate.MinSpawnHeight, Candidate.MaxSpawnHeight);
		WorldLocation.Z = GetActorLocation().Z + FloorTopZ + RandomHeight;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 空のアクターをスポーン
	AActor* ObstacleActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), WorldLocation, FRotator::ZeroRotator, SpawnParams);

	if (ObstacleActor)
	{
		// StaticMeshComponentを作成してルートコンポーネントとして設定
		UStaticMeshComponent* ObstacleMeshComp = NewObject<UStaticMeshComponent>(ObstacleActor, UStaticMeshComponent::StaticClass(), TEXT("ObstacleMesh"));
		ObstacleMeshComp->SetStaticMesh(MeshToUse);
		ObstacleMeshComp->SetWorldLocation(WorldLocation);
		ObstacleMeshComp->SetWorldScale3D(ObstacleScale);
		ObstacleMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ObstacleMeshComp->SetCollisionObjectType(ECC_WorldStatic);
		ObstacleMeshComp->RegisterComponent();

		// ルートコンポーネントとして設定
		ObstacleActor->SetRootComponent(ObstacleMeshComp);

		// 生成されたアクターを配列に追加
		GeneratedActors.Add(ObstacleActor);

#if WITH_EDITOR
		ObstacleActor->SetFolderPath(ObstacleFolderName);
#endif

		UE_LOG(LogTemp, Log, TEXT("[SpawnStaticMeshObstacle] Spawned at world (%.1f, %.1f, %.1f) scale (%.2f, %.2f, %.2f) Grounded=%s"),
			WorldLocation.X, WorldLocation.Y, WorldLocation.Z,
			ObstacleScale.X, ObstacleScale.Y, ObstacleScale.Z,
			Candidate.bGrounded ? TEXT("true") : TEXT("false"));
	}
}

void ACPP_ProceduralChunk::SpawnBPActorObstacle(const FObstacleCandidate& Candidate, const FVector& Location, float FloorTopZ)
{
	if (!Candidate.ActorClass) return;

	// Location は既にチャンク内の相対座標
	FVector LocalLocation = Location;

	// Z座標の計算
	if (Candidate.bGrounded)
	{
		// 床に接地
		LocalLocation.Z = FloorTopZ;
	}
	else
	{
		// 空中に配置: 床の上面からランダムな高さに配置
		float RandomHeight = RandomStream.FRandRange(Candidate.MinSpawnHeight, Candidate.MaxSpawnHeight);
		LocalLocation.Z = FloorTopZ + RandomHeight;
	}

	// ローカル座標からワールド座標に変換
	FVector WorldLocation = GetActorLocation() + LocalLocation;

	// ランダムサイズを計算
	FVector RandomScale = FVector(
		RandomStream.FRandRange(Candidate.MinSize.X, Candidate.MaxSize.X) / 100.0f,
		RandomStream.FRandRange(Candidate.MinSize.Y, Candidate.MaxSize.Y) / 100.0f,
		RandomStream.FRandRange(Candidate.MinSize.Z, Candidate.MaxSize.Z) / 100.0f
	);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		Candidate.ActorClass,
		WorldLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (SpawnedActor)
	{
		// ランダムスケールを適用
		SpawnedActor->SetActorScale3D(RandomScale);

		GeneratedActors.Add(SpawnedActor);

#if WITH_EDITOR
		SpawnedActor->SetFolderPath(ObstacleFolderName);
#endif

		UE_LOG(LogTemp, Log, TEXT("[SpawnBPActorObstacle] Chunk(%.0f,%.0f) World(%.1f,%.1f,%.1f) Scale(%.2f,%.2f,%.2f) Grounded=%s"),
			ChunkCoordinate.X, ChunkCoordinate.Y,
			WorldLocation.X, WorldLocation.Y, WorldLocation.Z,
			RandomScale.X, RandomScale.Y, RandomScale.Z,
			Candidate.bGrounded ? TEXT("true") : TEXT("false"));
	}
}