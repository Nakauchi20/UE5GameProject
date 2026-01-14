// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_BaseChunkManager.h"

ACPP_BaseChunkManager::ACPP_BaseChunkManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACPP_BaseChunkManager::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_BaseChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPP_BaseChunkManager::DisappearRandomFloors(int32 Count)
{
	// 基底クラスでは何もしない（派生クラスでオーバーライド）
	UE_LOG(LogTemp, Warning, TEXT("[BaseChunkManager] DisappearRandomFloors called but not implemented"));
}

void ACPP_BaseChunkManager::SpawnFireOnRandomFloors(int32 Count)
{
	// 基底クラスでは何もしない（派生クラスでオーバーライド）
	UE_LOG(LogTemp, Warning, TEXT("[BaseChunkManager] SpawnFireOnRandomFloors called but not implemented"));
}

TArray<UStaticMeshComponent*> ACPP_BaseChunkManager::GetAllFloorTiles() const
{
	// 基底クラスでは空配列を返す（派生クラスでオーバーライド）
	return TArray<UStaticMeshComponent*>();
}