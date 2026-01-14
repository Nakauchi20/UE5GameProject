#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "CPP_GameplayTagLibrary.generated.h"

UCLASS()
class MYEDEN3_API UCPP_GameplayTagLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // State Tags
    UFUNCTION(BlueprintPure, Category = "GameplayTags|State|Weapon")
    static FGameplayTag GetTag_State_Weapon_Reloading();

    UFUNCTION(BlueprintPure, Category = "GameplayTags|State|Weapon")
    static FGameplayTag GetTag_State_Weapon_Firing();

    UFUNCTION(BlueprintPure, Category = "GameplayTags|State|Character")
    static FGameplayTag GetTag_State_Character_Sliding();

    // 他のタグも必要に応じて追加...
};