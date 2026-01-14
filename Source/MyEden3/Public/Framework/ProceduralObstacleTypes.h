// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "ProceduralObstacleTypes.generated.h"

/**
 * 障害物候補の種類
 */
UENUM(BlueprintType)
enum class EObstacleType : uint8
{
	StaticMesh		UMETA(DisplayName = "Static Mesh"),
	BPActor			UMETA(DisplayName = "Blueprint Actor")
};

/**
 * 個別の障害物候補（スタティックメッシュまたはBPアクター）
 */
USTRUCT(BlueprintType)
struct FObstacleCandidate
{
	GENERATED_BODY()

	// 障害物の種類
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	EObstacleType ObstacleType = EObstacleType::StaticMesh;

	// スタティックメッシュ（ObstacleType が StaticMesh の場合）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (EditCondition = "ObstacleType == EObstacleType::StaticMesh", EditConditionHides))
	UStaticMesh* StaticMesh = nullptr;

	// BPアクタークラス（ObstacleType が BPActor の場合）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (EditCondition = "ObstacleType == EObstacleType::BPActor", EditConditionHides))
	TSubclassOf<AActor> ActorClass;

	// 選択される重み（大きいほど選ばれやすい）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ClampMin = "0"))
	float Weight = 100.0f;

	// サイズのランダム範囲（両タイプ共通）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Size")
	FVector MinSize = FVector(100, 100, 100);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Size")
	FVector MaxSize = FVector(300, 300, 200);

	// 床に接地するか（trueなら床の上面に配置、falseなら高さを指定）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Placement")
	bool bGrounded = true;

	// 生成する最小高さ（床からのオフセット、bGrounded=false の場合のみ）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Placement", meta = (EditCondition = "!bGrounded", EditConditionHides, ClampMin = "0"))
	float MinSpawnHeight = 100.0f;

	// 生成する最大高さ（床からのオフセット、bGrounded=false の場合のみ）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Placement", meta = (EditCondition = "!bGrounded", EditConditionHides, ClampMin = "0"))
	float MaxSpawnHeight = 500.0f;
};

/**
 * 障害物スポーングループ
 */
USTRUCT(BlueprintType)
struct FObstacleSpawnGroup
{
	GENERATED_BODY()

	// グループ名（識別用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FString GroupName = "Group";

	// このグループを使用する確率（0.0 〜 1.0）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnChance = 1.0f;

	// 生成する障害物の最小数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (ClampMin = "0"))
	int32 MinCount = 7;

	// 生成する障害物の最大数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (ClampMin = "0"))
	int32 MaxCount = 12;

	// 障害物候補のリスト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	TArray<FObstacleCandidate> Candidates;
};