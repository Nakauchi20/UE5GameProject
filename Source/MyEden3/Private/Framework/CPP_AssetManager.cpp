// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CPP_AssetManager.h"
#include "GAS/CPP_GameplayTags.h"

UCPP_AssetManager& UCPP_AssetManager::Get()
{
	UCPP_AssetManager* AssetManager = Cast<UCPP_AssetManager>(GEngine->AssetManager);
	if (AssetManager)
	{
		return *AssetManager;
	}

	UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManager class in Project Settings. Must be CPP_AssetManager"));
	return *NewObject<UCPP_AssetManager>();
}
