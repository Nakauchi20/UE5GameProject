// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CPP_OverlayData.generated.h"

/**
 * 
 */
UCLASS()
class MYEDEN3_API UCPP_OverlayData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
    FString Key;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
    TSubclassOf<class UAnimInstance> OverlayABP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
    TSubclassOf<class AActor> WeaponBP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
    FName SocketName;

    // GetLayerDataä÷êîÇí«â¡
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Overlay")
    void GetLayerData(
        FText& OutDisplayName,
        FString& OutKey,
        TSubclassOf<UAnimInstance>& OutOverlayABP,
        TSubclassOf<AActor>& OutWeaponBP,
        FName& OutSocketName
    ) const;

    // AssetManagerópÇÃïKê{ä÷êî
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
