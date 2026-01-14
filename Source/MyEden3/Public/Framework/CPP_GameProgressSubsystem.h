// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/CPP_BaseLevelSubsystem.h"
#include "CPP_GameProgressSubsystem.generated.h"

/**
 * ゲーム進行管理用のワールドサブシステム
 * 時間管理、イベント発火タイミングの制御を担当
 */
UCLASS()
class MYEDEN3_API UCPP_GameProgressSubsystem : public UCPP_BaseLevelSubsystem
{
	GENERATED_BODY()

public:
	// ========== Subsystem Lifecycle ==========
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Initialization ==========
	virtual void InitializeLevelSubsystem(ACPP_BaseChunkManager* InChunkManager) override;

	// 互換性のために残す（既存のコードで使われている可能性）
	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	void InitializeGameProgress(ACPP_BaseChunkManager* InChunkManager);

	// ========== Game State ==========
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Progress")
	float GetGameElapsedTime() const { return GameElapsedTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Progress")
	bool IsGameActive() const { return bGameActive; }

	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	void EndGame();

	// ========== Player Death Notification ==========
	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	void NotifyPlayerDied();

protected:
	// ========== Update Function (Timer) ==========
	void UpdateGameProgress();

	// ========== Floor Events ==========
	void CheckAndTriggerFloorEvents();
	void TriggerFloorDisappearEvent();
	void TriggerFloorFireEvent();

	// ========== Game State ==========
	UPROPERTY(BlueprintReadOnly, Category = "Game Progress")
	bool bGameActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Game Progress")
	float GameElapsedTime = 0.0f;

	// ========== Update Settings ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Progress")
	float UpdateInterval = 0.1f;

	// ========== Event Timing Settings ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events|Floor")
	float FloorDisappearStartTime = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events|Floor")
	float FloorDisappearInterval = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events|Floor")
	int32 FloorDisappearCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events|Floor")
	float FloorFireStartTime = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events|Floor")
	float FloorFireInterval = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events|Floor")
	int32 FloorFireCount = 3;

private:
	// ========== Event Tracking ==========
	float LastFloorDisappearTime = 0.0f;
	float LastFloorFireTime = 0.0f;

	// ========== Timer ==========
	FTimerHandle UpdateTimerHandle;
};