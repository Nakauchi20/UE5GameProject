// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/AssetManager.h"
#include "Framework/CPP_OverlayData.h"
#include "CPP_OverlayDataManager.generated.h"

/**
 * 
 */
UCLASS()
class MYEDEN3_API UCPP_OverlayDataManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Overlay")
    UCPP_OverlayData* GetOverlayData(FName OverlayName) const;

    UFUNCTION(BlueprintCallable, Category = "Overlay")
    TArray<UCPP_OverlayData*> GetAllOverlayData() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Overlay", meta = (WorldContext = "WorldContextObject"))
    static UCPP_OverlayDataManager* Get(const UObject* WorldContextObject);

private:
    UPROPERTY()
    TMap<FName, UCPP_OverlayData*> OverlayDataMap;

    void LoadAllOverlayData();
};
