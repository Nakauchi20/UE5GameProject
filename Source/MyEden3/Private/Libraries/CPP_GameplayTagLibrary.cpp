#include "Libraries/CPP_GameplayTagLibrary.h"
#include "GAS/CPP_GameplayTags.h"

FGameplayTag UCPP_GameplayTagLibrary::GetTag_State_Weapon_Reloading()
{
    return FCPP_GameplayTags::Get().State_Weapon_Reloading;
}

FGameplayTag UCPP_GameplayTagLibrary::GetTag_State_Weapon_Firing()
{
    return FCPP_GameplayTags::Get().State_Weapon_Firing;
}

FGameplayTag UCPP_GameplayTagLibrary::GetTag_State_Character_Sliding()
{
    return FCPP_GameplayTags::Get().State_Movement_Sliding;
}