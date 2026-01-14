#include "GAS/CPP_GameplayTags.h"
#include "GameplayTagsManager.h"

// 静的インスタンス - これによりUE5が自動的にAddTags()を呼び出す
static FCPP_GameplayTags GameplayTagsInstance;

FCPP_GameplayTags& FCPP_GameplayTags::Get()
{
    return GameplayTagsInstance;
}

void FCPP_GameplayTags::AddTags()
{
    UE_LOG(LogTemp, Log, TEXT("[FCPP_GameplayTags] AddTags() called - Registering Native GameplayTags"));

    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // --- Ability Tags ---
    Ability_Weapon_Fire = Manager.AddNativeGameplayTag(FName("Ability.Weapon.Fire"), TEXT("Ability to fire weapon"));
    Ability_Weapon_Reload = Manager.AddNativeGameplayTag(FName("Ability.Weapon.Reload"), TEXT("Ability to reload weapon"));
    Ability_Weapon_Switch = Manager.AddNativeGameplayTag(FName("Ability.Weapon.Switch"), TEXT("Ability to switch weapon"));

    Ability_Movement_Jump = Manager.AddNativeGameplayTag(FName("Ability.Movement.Jump"), TEXT("Ability to jump"));
    Ability_Movement_Walk = Manager.AddNativeGameplayTag(FName("Ability.Movement.Walk"), TEXT("Ability to walk"));
    Ability_Movement_Sprint = Manager.AddNativeGameplayTag(FName("Ability.Movement.Sprint"), TEXT("Ability to sprint"));
    Ability_Movement_Crouch = Manager.AddNativeGameplayTag(FName("Ability.Movement.Crouch"), TEXT("Ability to crouch"));
    Ability_Movement_Slide = Manager.AddNativeGameplayTag(FName("Ability.Movement.Slide"), TEXT("Ability to slide"));

    Ability_Character_Aim = Manager.AddNativeGameplayTag(FName("Ability.Character.Aim"), TEXT("Ability to aim (ADS)"));

    // --- State Tags ---
    State_Weapon_Firing = Manager.AddNativeGameplayTag(FName("State.Weapon.Firing"), TEXT("Currently firing"));
    State_Weapon_Reloading = Manager.AddNativeGameplayTag(FName("State.Weapon.Reloading"), TEXT("Currently reloading"));

    State_Movement_Walking = Manager.AddNativeGameplayTag(FName("State.Movement.Walking"), TEXT("Currently walking"));
    State_Movement_Sprinting = Manager.AddNativeGameplayTag(FName("State.Movement.Sprinting"), TEXT("Currently sprinting"));
    State_Movement_Crouching = Manager.AddNativeGameplayTag(FName("State.Movement.Crouching"), TEXT("Currently crouching"));
    State_Movement_Sliding = Manager.AddNativeGameplayTag(FName("State.Movement.Sliding"), TEXT("Currently sliding"));
    State_Movement_InAir = Manager.AddNativeGameplayTag(FName("State.Movement.InAir"), TEXT("Character is currently in the air"));

    State_Character_Aiming = Manager.AddNativeGameplayTag(FName("State.Character.Aiming"), TEXT("Currently aiming (ADS)"));
    State_Character_Switching = Manager.AddNativeGameplayTag(FName("State.Character.Switching"), TEXT("Currently switching weapon"));
    State_Character_Stunned = Manager.AddNativeGameplayTag(FName("State.Character.Stunned"), TEXT("Character is stunned"));
    State_Character_Dead = Manager.AddNativeGameplayTag(FName("State.Character.Dead"), TEXT("Character is dead"));

    // --- Event Tags ---
    Event_Weapon_FireComplete = Manager.AddNativeGameplayTag(FName("Event.Weapon.FireComplete"), TEXT("Notification for Fire completion"));
    Event_Weapon_ReloadComplete = Manager.AddNativeGameplayTag(FName("Event.Weapon.ReloadComplete"), TEXT("Notification for reload completion"));
    Event_Weapon_SwitchComplete = Manager.AddNativeGameplayTag(FName("Event.Weapon.SwitchComplete"), TEXT("Notification for weapon switch completion"));

    Event_Movement_Jumped = Manager.AddNativeGameplayTag(FName("Event.Movement.Jumped"), TEXT("Event triggered at the moment of jumping"));
    Event_Movement_Landed = Manager.AddNativeGameplayTag(FName("Event.Movement.Landed"), TEXT("Event triggered at the moment of Landed"));
    Event_Movement_CrouchComplete = Manager.AddNativeGameplayTag(FName("Event.Movement.CrouchComplete"), TEXT("Notification for crouch completion"));

    Event_Character_AimComplete = Manager.AddNativeGameplayTag(FName("Event.Character.AimComplete"), TEXT("Notification for aim completion"));
    Event_Character_HitReaction = Manager.AddNativeGameplayTag(FName("Event.Character.HitReaction"), TEXT("Notification for being hit"));

    // --- Damage Tags ---
    Damage_Type_Physical = Manager.AddNativeGameplayTag(FName("Damage.Type.Physical"), TEXT("Physical damage type"));
    Damage_Type_Fire = Manager.AddNativeGameplayTag(FName("Damage.Type.Fire"), TEXT("Fire damage type"));
    Damage_Type_Explosive = Manager.AddNativeGameplayTag(FName("Damage.Type.Explosive"), TEXT("Explosive damage type"));
    Damage_Hit_Headshot = Manager.AddNativeGameplayTag(FName("Damage.Hit.Headshot"), TEXT("Damage modifier for headshots"));

    UE_LOG(LogTemp, Log, TEXT("[FCPP_GameplayTags] AddTags() completed - %s"),
        *State_Movement_InAir.ToString());
}