// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CPP_BaseLevelSubsystem.h"
#include "Framework/CPP_BaseChunkManager.h"

void UCPP_BaseLevelSubsystem::InitializeLevelSubsystem(ACPP_BaseChunkManager* InChunkManager)
{
	ChunkManager = InChunkManager;

	if (ChunkManager)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Initialized with ChunkManager"), *GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Initialized without ChunkManager"), *GetClass()->GetName());
	}
}