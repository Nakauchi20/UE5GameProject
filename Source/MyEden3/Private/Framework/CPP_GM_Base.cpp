// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_GM_Base.h"
#include "Characters/CPP_CharacterBase.h"
#include "Framework/CPP_PlayerStateBase.h"
#include "Framework/CPP_PC_Base.h"
#include "Framework/CPP_BaseChunkManager.h"
#include "Framework/CPP_ChunkManager.h"
#include "Framework/CPP_StaticChunkManager.h"
#include "Framework/CPP_GameProgressSubsystem.h"
#include "Framework/CPP_BaseLevelSubsystem.h"
#include "Data/CPP_LevelConfigDataAsset.h"

ACPP_GM_Base::ACPP_GM_Base()
{
	DefaultPawnClass = ACPP_CharacterBase::StaticClass();
	PlayerStateClass = ACPP_PlayerStateBase::StaticClass();
}

void ACPP_GM_Base::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("GameMode BeginPlay Called"));

	InitializeLevelFromConfig();
}

void ACPP_GM_Base::InitializeLevelFromConfig()
{
	if (!LevelConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] LevelConfig is not set!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Loading Level Config: %s"), *LevelConfig->LevelName.ToString());

	// チャンクマネージャーをスポーン
	ChunkManager = SpawnChunkManagerFromConfig(LevelConfig);

	if (ChunkManager)
	{
		// 複数のSubsystemを初期化
		InitializeSubsystems(LevelConfig, ChunkManager);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] Failed to spawn ChunkManager"));
	}
}

ACPP_BaseChunkManager* ACPP_GM_Base::SpawnChunkManagerFromConfig(UCPP_LevelConfigDataAsset* Config)
{
	if (!Config->ChunkManagerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] ChunkManagerClass is not set in LevelConfig"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	ACPP_BaseChunkManager* NewChunkManager = GetWorld()->SpawnActor<ACPP_BaseChunkManager>(
		Config->ChunkManagerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!NewChunkManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] Failed to spawn ChunkManager"));
		return nullptr;
	}

	// 共通設定を適用
	NewChunkManager->WorldSeed = Config->WorldSeed;
	NewChunkManager->HexagonSize = 500.0f;

	// 動的チャンクマネージャーの場合
	if (NewChunkManager->IsDynamicChunkManager())
	{
		if (ACPP_ChunkManager* DynamicManager = Cast<ACPP_ChunkManager>(NewChunkManager))
		{
			DynamicManager->ChunkLoadRadius = Config->ChunkLoadRadius;
			DynamicManager->ChunkUnloadRadius = Config->ChunkUnloadRadius;
			DynamicManager->UpdateInterval = Config->UpdateInterval;

			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Dynamic ChunkManager configured"));
		}
	}
	// 静的チャンクマネージャーの場合
	else
	{
		if (ACPP_StaticChunkManager* StaticManager = Cast<ACPP_StaticChunkManager>(NewChunkManager))
		{
			StaticManager->GenerationRadius = Config->StaticGenerationRadius;
			StaticManager->bGenerateAroundPlayer = Config->bGenerateAroundPlayer;
			StaticManager->FixedGenerationLocation = Config->FixedGenerationLocation;

			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Static ChunkManager configured"));
		}
	}

	return NewChunkManager;
}

// 複数のSubsystemを初期化
void ACPP_GM_Base::InitializeSubsystems(UCPP_LevelConfigDataAsset* Config, ACPP_BaseChunkManager* InChunkManager)
{
	if (Config->SubsystemClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] No Subsystems specified in LevelConfig"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Initializing %d Subsystem(s)"), Config->SubsystemClasses.Num());

	for (TSubclassOf<UCPP_BaseLevelSubsystem> SubsystemClass : Config->SubsystemClasses)
	{
		if (!SubsystemClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Null SubsystemClass found, skipping"));
			continue;
		}

		// GetSubsystemBase を使用
		UWorldSubsystem* WorldSubsystem = GetWorld()->GetSubsystemBase(SubsystemClass);

		if (!WorldSubsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[GameMode] Failed to get Subsystem: %s"),
				*SubsystemClass->GetName());
			continue;
		}

		// BaseLevelSubsystemにキャスト
		UCPP_BaseLevelSubsystem* Subsystem = Cast<UCPP_BaseLevelSubsystem>(WorldSubsystem);

		if (!Subsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[GameMode] Subsystem is not a BaseLevelSubsystem: %s"),
				*SubsystemClass->GetName());
			continue;
		}

		// 初期化
		Subsystem->InitializeLevelSubsystem(InChunkManager);
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Initialized Subsystem: %s"),
			*SubsystemClass->GetName());
	}
}