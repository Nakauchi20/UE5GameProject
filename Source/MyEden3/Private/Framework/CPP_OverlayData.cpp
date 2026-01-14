// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CPP_OverlayData.h"

FPrimaryAssetId UCPP_OverlayData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("OverlayData", GetFName());
}

void UCPP_OverlayData::GetLayerData(
    FText& OutDisplayName,
    FString& OutKey,
    TSubclassOf<UAnimInstance>& OutOverlayABP,
    TSubclassOf<AActor>& OutWeaponBP,
    FName& OutSocketName
) const
{
    OutDisplayName = DisplayName;
    OutKey = Key;
    OutOverlayABP = OverlayABP;
    OutWeaponBP = WeaponBP;
    OutSocketName = SocketName;
}