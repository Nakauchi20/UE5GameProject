#include "Framework/CPP_OverlayDataManager.h"
#include "Framework/CPP_OverlayData.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UCPP_OverlayDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Error, TEXT("!!! CPP_OverlayDataManager Initialize START !!!"));

    LoadAllOverlayData();

    UE_LOG(LogTemp, Error, TEXT("!!! CPP_OverlayDataManager Initialize END !!!"));
}

void UCPP_OverlayDataManager::LoadAllOverlayData()
{
    UE_LOG(LogTemp, Error, TEXT("!!! LoadAllOverlayData START !!!"));

    // ★ 手動でフォルダをスキャン
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> AssetDataList;
    FARFilter Filter;
    Filter.ClassPaths.Add(UCPP_OverlayData::StaticClass()->GetClassPathName());
    Filter.PackagePaths.Add("/Game/OverlaySystem/Data");
    Filter.bRecursivePaths = true;

    AssetRegistry.GetAssets(Filter, AssetDataList);

    UE_LOG(LogTemp, Error, TEXT("!!! Found %d CPP_OverlayData assets in folder !!!"), AssetDataList.Num());

    for (const FAssetData& AssetData : AssetDataList)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found in folder: %s"), *AssetData.AssetName.ToString());

        UCPP_OverlayData* LoadedData = Cast<UCPP_OverlayData>(AssetData.GetAsset());
        if (LoadedData)
        {
            OverlayDataMap.Add(LoadedData->GetFName(), LoadedData);
            UE_LOG(LogTemp, Error, TEXT("✓ Loaded: %s"), *LoadedData->GetName());
        }
    }

    UE_LOG(LogTemp, Error, TEXT("!!! Total loaded: %d !!!"), OverlayDataMap.Num());
}

UCPP_OverlayData* UCPP_OverlayDataManager::GetOverlayData(FName OverlayName) const
{
    if (UCPP_OverlayData* const* FoundData = OverlayDataMap.Find(OverlayName))
    {
        return *FoundData;
    }

    UE_LOG(LogTemp, Warning, TEXT("OverlayData not found: %s"), *OverlayName.ToString());
    return nullptr;
}

TArray<UCPP_OverlayData*> UCPP_OverlayDataManager::GetAllOverlayData() const
{
    TArray<UCPP_OverlayData*> AllData;
    OverlayDataMap.GenerateValueArray(AllData);
    return AllData;
}

UCPP_OverlayDataManager* UCPP_OverlayDataManager::Get(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UCPP_OverlayDataManager>();
        }
    }
    return nullptr;
}