// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_GameProgressSubsystem.h"
#include "Framework/CPP_BaseChunkManager.h"
#include "Framework/CPP_StaticChunkManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UCPP_GameProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("[GameProgressSubsystem] Initialized"));
}

void UCPP_GameProgressSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("[GameProgressSubsystem] Deinitialized"));
}

void UCPP_GameProgressSubsystem::InitializeLevelSubsystem(ACPP_BaseChunkManager* InChunkManager)
{
	// 基底クラスの初期化を呼び出す
	Super::InitializeLevelSubsystem(InChunkManager);

	// ゲーム進行固有の初期化
	if (ChunkManager)
	{
		StartGame();
	}
}

// 互換性のために残す
void UCPP_GameProgressSubsystem::InitializeGameProgress(ACPP_BaseChunkManager* InChunkManager)
{
	// 新しい関数に処理を委譲
	InitializeLevelSubsystem(InChunkManager);
}

void UCPP_GameProgressSubsystem::StartGame()
{
	if (bGameActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameProgressSubsystem] Game already started"));
		return;
	}

	bGameActive = true;
	GameElapsedTime = 0.0f;
	LastFloorDisappearTime = 0.0f;
	LastFloorFireTime = 0.0f;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			UpdateTimerHandle,
			this,
			&UCPP_GameProgressSubsystem::UpdateGameProgress,
			UpdateInterval,
			true
		);

		UE_LOG(LogTemp, Log, TEXT("[GameProgressSubsystem] Game Started! Timer started with interval: %.2f"), UpdateInterval);
	}
}

void UCPP_GameProgressSubsystem::EndGame()
{
	if (!bGameActive)
	{
		return;
	}

	bGameActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameProgressSubsystem] Game Ended. Total Time: %.2f seconds"), GameElapsedTime);
}

void UCPP_GameProgressSubsystem::NotifyPlayerDied()
{
	UE_LOG(LogTemp, Warning, TEXT("[GameProgressSubsystem] Player died at %.2f seconds"), GameElapsedTime);
	EndGame();

	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			UE_LOG(LogTemp, Log, TEXT("[GameProgressSubsystem] Notified GameMode of player death"));
		}
	}
}

void UCPP_GameProgressSubsystem::UpdateGameProgress()
{
	if (!bGameActive)
	{
		return;
	}

	GameElapsedTime += UpdateInterval;
	CheckAndTriggerFloorEvents();
}

void UCPP_GameProgressSubsystem::CheckAndTriggerFloorEvents()
{
	if (!ChunkManager)
	{
		return;
	}

	// 床消滅イベントのチェック
	if (GameElapsedTime >= FloorDisappearStartTime)
	{
		float TimeSinceLastDisappear = GameElapsedTime - LastFloorDisappearTime;
		if (LastFloorDisappearTime == 0.0f || TimeSinceLastDisappear >= FloorDisappearInterval)
		{
			TriggerFloorDisappearEvent();
			LastFloorDisappearTime = GameElapsedTime;
		}
	}

	// 床から火が出るイベントのチェック
	if (GameElapsedTime >= FloorFireStartTime)
	{
		float TimeSinceLastFire = GameElapsedTime - LastFloorFireTime;
		if (LastFloorFireTime == 0.0f || TimeSinceLastFire >= FloorFireInterval)
		{
			TriggerFloorFireEvent();
			LastFloorFireTime = GameElapsedTime;
		}
	}
}

void UCPP_GameProgressSubsystem::TriggerFloorDisappearEvent()
{
	if (!ChunkManager)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameProgressSubsystem] Triggering Floor Disappear Event - Count: %d"), FloorDisappearCount);
	ChunkManager->DisappearRandomFloors(FloorDisappearCount);
}

void UCPP_GameProgressSubsystem::TriggerFloorFireEvent()
{
	if (!ChunkManager)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameProgressSubsystem] Triggering Floor Fire Event - Count: %d"), FloorFireCount);
	ChunkManager->SpawnFireOnRandomFloors(FloorFireCount);
}