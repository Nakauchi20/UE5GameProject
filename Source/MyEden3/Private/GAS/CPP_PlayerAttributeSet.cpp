// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CPP_PlayerAttributeSet.h"
#include "Characters/CPP_CharacterBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h" 
#include "Characters/CPP_CharacterMovementComponent.h"


UCPP_PlayerAttributeSet::UCPP_PlayerAttributeSet()
    : MaxSpeed(600.0f), MaxSpeedCrouch(200.0f)
    , MaxHealth(100.0f), Health(100.0f)
{
}

void UCPP_PlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, MaxSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, MaxSpeedCrouch, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

void UCPP_PlayerAttributeSet::OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, MaxSpeed, OldMaxSpeed);
    NotifyCharacterOfSpeedChange(GetMaxSpeed(), false);
}

void UCPP_PlayerAttributeSet::OnRep_MaxSpeedCrouch(const FGameplayAttributeData& OldMaxSpeedCrouch)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, MaxSpeedCrouch, OldMaxSpeedCrouch);
    NotifyCharacterOfSpeedChange(GetMaxSpeedCrouch(), true);
}

void UCPP_PlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, MaxHealth, OldMaxHealth);
}

void UCPP_PlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, Health, OldHealth);
}

FGameplayAttribute UCPP_PlayerAttributeSet::MaxSpeedAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(UCPP_PlayerAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCPP_PlayerAttributeSet, MaxSpeed));
    return FGameplayAttribute(Property);
}

FGameplayAttribute UCPP_PlayerAttributeSet::MaxSpeedCrouchAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(UCPP_PlayerAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCPP_PlayerAttributeSet, MaxSpeedCrouch));
    return FGameplayAttribute(Property);
}

FGameplayAttribute UCPP_PlayerAttributeSet::MaxHealthAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(UCPP_PlayerAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCPP_PlayerAttributeSet, MaxHealth));
    return FGameplayAttribute(Property);
}

FGameplayAttribute UCPP_PlayerAttributeSet::HealthAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(UCPP_PlayerAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCPP_PlayerAttributeSet, Health));
    return FGameplayAttribute(Property);
}

//GameplayEffect によって属性が変化した後に呼び出されます。
void UCPP_PlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;

    if (Attribute == MaxSpeedAttribute())
    {
        #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("MaxSpeed changed to: %.2f"), GetMaxSpeed());
        #endif
        NotifyCharacterOfSpeedChange(GetMaxSpeed(), false);
    }
    else if (Attribute == MaxSpeedCrouchAttribute())
    {
        #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("MaxSpeedCrouch changed to: %.2f"), GetMaxSpeedCrouch());
        #endif
        NotifyCharacterOfSpeedChange(GetMaxSpeedCrouch(), true);
    }
    else if (Attribute == HealthAttribute())
    {
        float DeltaValue = 0.0f;
        if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
        {
            DeltaValue = Data.EvaluatedData.Magnitude;
        }
        HandleHealthAttributeChange(Data, DeltaValue);
    }
    else if (Attribute == MaxHealthAttribute())
    {
        // MaxHealth変更時にHealthを調整
        if (GetHealth() > GetMaxHealth())
        {
            SetHealth(GetMaxHealth());
        }
    }
}

void UCPP_PlayerAttributeSet::NotifyCharacterOfSpeedChange(float NewSpeed, bool bIsCrouchSpeed)
{
    if (AActor* Owner = GetOwningActor())
    {
        ACPP_CharacterBase* Character = nullptr;
        
        // PlayerStateの場合、実際のCharacterを取得する必要がある
        if (APawn* OwnerPawn = Cast<APawn>(Owner))
        {
            Character = Cast<ACPP_CharacterBase>(OwnerPawn);
        }
        // PlayerStateを通してCharacterを取得
        else if (APlayerState* PS = Cast<APlayerState>(Owner))
        {
            if (APawn* Pawn = PS->GetPawn())
            {
                Character = Cast<ACPP_CharacterBase>(Pawn);
            }
        }

        if (Character)
        {
            // 実際のしゃがみ状態を取得
            bool bIsCurrentlyCrouching = false;
            if (UCPP_CharacterMovementComponent* MovementComp = Character->GetCustomMovementComponent())
            {
                bIsCurrentlyCrouching = MovementComp->IsCrouching();
            }

            if (bIsCrouchSpeed)
            {
                Character->HandleMaxSpeedCrouchChanged(NewSpeed);
            }
            else
            {
                Character->HandleMaxSpeedChanged(NewSpeed);
            }
        }
    }
}

void UCPP_PlayerAttributeSet::HandleHealthAttributeChange(const FGameplayEffectModCallbackData& Data, float DeltaValue)
{
    const float PreviousHealth = GetHealth() - DeltaValue;

    // Health制限
    const float NewHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
    SetHealth(NewHealth);

    // ターゲット情報取得
    ACPP_CharacterBase* TargetCharacter = nullptr;
    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetCharacter = Cast<ACPP_CharacterBase>(Data.Target.AbilityActorInfo->AvatarActor.Get());
    }

    if (!TargetCharacter || TargetCharacter->IsDead())
    {
        return;
    }

    // ダメージ処理
    if (DeltaValue < 0.0f)
    {
        const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
        const float ActualDamage = FMath::Abs(DeltaValue);
        TargetCharacter->HandleDamageReceived(ActualDamage, SourceTags);
    }

    // 死亡判定
    if (GetHealth() <= 0.0f && PreviousHealth > 0.0f)
    {
        TargetCharacter->HandleDeath();
    }
}