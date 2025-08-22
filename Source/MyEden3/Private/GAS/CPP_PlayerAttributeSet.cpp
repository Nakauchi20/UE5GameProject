// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CPP_PlayerAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"		// PostGameplayEffectExecuteのContextデータなどを扱うのに必要
#include "Characters/CPP_CharacterBase.h"
#include "Net/UnrealNetwork.h"


/** コンストラクタにより初期値設定 */
UCPP_PlayerAttributeSet::UCPP_PlayerAttributeSet()
    : MaxSpeed(600.0f)
    , MaxHealth(100.0f), Health(100.0f)
{
}

// レプリケーション設定
void UCPP_PlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, MaxSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCPP_PlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

// MaxSpeed レプリケーション通知
void UCPP_PlayerAttributeSet::OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, MaxSpeed, OldMaxSpeed);

    // クライアント側でも速度更新を実行
    if (AActor* Owner = GetOwningActor())
    {
        if (ACPP_CharacterBase* Character = Cast<ACPP_CharacterBase>(Owner))
        {
            Character->HandleMaxSpeedChanged(GetMaxSpeed());
            UE_LOG(LogTemp, Warning, TEXT("Client: MaxSpeed replicated to %.2f"), GetMaxSpeed());
        }
    }
}

// MaxHealth レプリケーション通知
void UCPP_PlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, MaxHealth, OldMaxHealth);

    UE_LOG(LogTemp, Log, TEXT("MaxHealth replicated: %.2f -> %.2f"),
        OldMaxHealth.GetCurrentValue(), GetMaxHealth());
}

// Health レプリケーション通知
void UCPP_PlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCPP_PlayerAttributeSet, Health, OldHealth);

    UE_LOG(LogTemp, Log, TEXT("Health replicated: %.2f -> %.2f"),
        OldHealth.GetCurrentValue(), GetHealth());
}

// アトリビュート型取得関数
FGameplayAttribute UCPP_PlayerAttributeSet::MaxSpeedAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(UCPP_PlayerAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCPP_PlayerAttributeSet, MaxSpeed));
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

    // 受け取ったデータから各種情報を取得
    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

    // GameplayEffectにより指定されたアトリビュート変化値を計算
    float DeltaValue = 0;
    if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
    {
        DeltaValue = Data.EvaluatedData.Magnitude;
    }

    // 受け取ったデータからターゲットアクター、コントローラ、キャラクタの取得
    AActor* TargetActor = nullptr;
    AController* TargetController = nullptr;
    ACPP_CharacterBase* TargetCharacter = nullptr;
    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
        TargetCharacter = Cast<ACPP_CharacterBase>(TargetActor);
    }

    //MaxSpeed属性の処理
    if (Data.EvaluatedData.Attribute == MaxSpeedAttribute())
    {
        if (TargetCharacter)
        {
            float NewSpeed = GetMaxSpeed(); // 現在値を取得
            TargetCharacter->HandleMaxSpeedChanged(NewSpeed);
            UE_LOG(LogTemp, Warning, TEXT("MaxSpeed attribute value: %.2f"), NewSpeed);
        }
    }

    // Health属性の処理（ダメージ/回復処理）
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // 変更前のHealth値を記録
        float PreviousHealth = GetHealth() - DeltaValue;

        // Health を 0 ～ MaxHealth に制限
        float NewHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
        SetHealth(NewHealth);

        // ダメージを受けた場合の処理（Healthが減少した場合）
        if (DeltaValue < 0.0f && TargetCharacter && !TargetCharacter->IsDead())
        {
            float ActualDamage = FMath::Abs(DeltaValue);
            TargetCharacter->HandleDamageReceived(ActualDamage, SourceTags);
        }

        // 死亡判定
        if (GetHealth() <= 0.0f && PreviousHealth > 0.0f)
        {
            if (TargetCharacter && !TargetCharacter->IsDead())
            {
                TargetCharacter->HandleDeath();
            }
        }

        // デバッグログ
        UE_LOG(LogTemp, Warning, TEXT("Health changed: %.2f -> %.2f (Delta: %.2f)"),
            PreviousHealth, GetHealth(), DeltaValue);
    }

    // MaxHealth属性の処理
    if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        // MaxHealthが変更された場合、現在のHealthも調整
        if (GetHealth() > GetMaxHealth())
        {
            SetHealth(GetMaxHealth());
        }
    }
}