#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

/**
 * FGameplayTagNativeAdder を継承してNativeタグを自動登録
 * UE5が適切なタイミングで AddTags() を呼び出す
 */
class MYEDEN3_API FCPP_GameplayTags : public FGameplayTagNativeAdder
{
public:

    static FCPP_GameplayTags& Get();
    FCPP_GameplayTags() {}

    // FGameplayTagNativeAdder interface
    virtual void AddTags() override;

    // ==========================================
    // ABILITY TAGS (行為: 入力によって起動するもの)
    // ==========================================
    FGameplayTag Ability_Weapon_Fire;
    FGameplayTag Ability_Weapon_Reload;
    FGameplayTag Ability_Weapon_Switch;

    FGameplayTag Ability_Movement_Jump;
    FGameplayTag Ability_Movement_Walk;
    FGameplayTag Ability_Movement_Sprint;
    FGameplayTag Ability_Movement_Crouch;
    FGameplayTag Ability_Movement_Slide;

    FGameplayTag Ability_Character_Aim;

    // ==========================================
    // STATE TAGS (状態: 進行中のアクションや制限用)
    // ==========================================
    FGameplayTag State_Weapon_Firing;
    FGameplayTag State_Weapon_Reloading;

    FGameplayTag State_Movement_Walking;
    FGameplayTag State_Movement_Sprinting;
    FGameplayTag State_Movement_Crouching;
    FGameplayTag State_Movement_Sliding;
    FGameplayTag State_Movement_InAir;

    FGameplayTag State_Character_Aiming;
    FGameplayTag State_Character_Switching;
    FGameplayTag State_Character_Stunned;
    FGameplayTag State_Character_Dead;

    // ==========================================
    // EVENT TAGS (通知: 特定の瞬間を伝えるもの)
    // ==========================================
    FGameplayTag Event_Weapon_FireComplete;
    FGameplayTag Event_Weapon_ReloadComplete;
    FGameplayTag Event_Weapon_SwitchComplete;

    FGameplayTag Event_Movement_Jumped;
    FGameplayTag Event_Movement_Landed;
    FGameplayTag Event_Movement_CrouchComplete;

    FGameplayTag Event_Character_AimComplete;
    FGameplayTag Event_Character_HitReaction;

    // ==========================================
    // DAMAGE TAGS (性質: ダメージタイプなど)
    // ==========================================
    FGameplayTag Damage_Type_Physical;
    FGameplayTag Damage_Type_Fire;
    FGameplayTag Damage_Type_Explosive;
    FGameplayTag Damage_Hit_Headshot;

};