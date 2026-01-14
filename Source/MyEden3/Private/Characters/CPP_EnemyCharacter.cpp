// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CPP_EnemyCharacter.h"
#include "GAS/CPP_PlayerAttributeSet.h"
#include "Data/CPP_EnemyStatsData.h"
#include "Characters/CPP_CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/StateTreeComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"

ACPP_EnemyCharacter::ACPP_EnemyCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // AbilitySystemComponentの作成
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    // レプリケーションモード設定（敵キャラクタ用）
    // Mixed: GameplayEffectsはサーバーのみ、GameplayCuesとGameplayTagsは全クライアント
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // AttributeSetの作成
    AttributeSet = CreateDefaultSubobject<UCPP_PlayerAttributeSet>(TEXT("AttributeSet"));

    // StateTreeComponentの作成
    StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void ACPP_EnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACPP_EnemyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeAbilitySystem();
}

void ACPP_EnemyCharacter::InitializeAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // サーバー側でのみ初期化
    if (HasAuthority())
    {
        // AbilitySystemComponentの初期化
        AbilitySystemComponent->InitAbilityActorInfo(this, this);

        // ステータス初期化
        if (EnemyStatsTable)
        {
            // DataTableからステータスを読み込み
            LoadStatsFromDataTable();
        }
        else
        {
            // DataTableが設定されていない場合はデフォルト値を使用
            UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter: No DataTable set, using default stats"));
            SetInitialStats(100.0f, 400.0f);
        }

        OnAbilitySystemInitializedEvent();
    }
}

// ============== Stats Configuration ==============
bool ACPP_EnemyCharacter::LoadStatsFromDataTable()
{
    if (!EnemyStatsTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter: DataTable is null"));
        return false;
    }

    // DataTableから敵のステータスを取得
    FEnemyStatsData* StatsData = EnemyStatsTable->FindRow<FEnemyStatsData>(EnemyStatsID, TEXT("LoadEnemyStats"));

    if (!StatsData)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyCharacter: Stats ID '%s' not found in DataTable"), *EnemyStatsID.ToString());
        return false;
    }

    // ステータスを適用
    SetInitialStats(StatsData->MaxHealth, StatsData->MaxSpeed);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("EnemyCharacter: Loaded stats for '%s' - HP:%.1f Speed:%.1f"),
        *StatsData->DisplayName.ToString(), StatsData->MaxHealth, StatsData->MaxSpeed);
#endif

    return true;
}

void ACPP_EnemyCharacter::SetInitialStats(float InMaxHealth, float InMaxSpeed)
{
    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyCharacter: AttributeSet is null"));
        return;
    }

    // Attributeの初期値を設定
    AttributeSet->SetMaxHealth(InMaxHealth);
    AttributeSet->SetHealth(InMaxHealth); // Healthは最大値に設定
    AttributeSet->SetMaxSpeed(InMaxSpeed);

    // MovementComponentへの反映を遅延実行
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, [this, InMaxSpeed]()
            {
                // MovementComponentが準備できてから速度を設定
                HandleMaxSpeedChanged(InMaxSpeed);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
                float ActualMovementSpeed = 0.0f;
                if (UCPP_CharacterMovementComponent* MovementComp = GetCustomMovementComponent())
                {
                    ActualMovementSpeed = MovementComp->GetMaxSpeed();
                    UE_LOG(LogTemp, Log, TEXT("EnemyCharacter: Movement speed applied - %.1f"), ActualMovementSpeed);
                }
#endif
            }, 0.1f, false);
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("EnemyCharacter: Set initial stats - HP:%.1f/%.1f AttributeSpeed:%.1f"),
        AttributeSet->GetHealth(), AttributeSet->GetMaxHealth(), AttributeSet->GetMaxSpeed());
#endif
}

// ============== Debug Functions ==============
FString ACPP_EnemyCharacter::GetDebugStatsInfo() const
{
    if (!AttributeSet)
    {
        return TEXT("AttributeSet is NULL");
    }

    // MovementComponentの速度も確認
    float CurrentMovementSpeed = 0.0f;
    if (UCPP_CharacterMovementComponent* MovementComp = GetCustomMovementComponent())
    {
        CurrentMovementSpeed = MovementComp->GetMaxSpeed();
    }

    return FString::Printf(
        TEXT("Enemy: %s\nHP: %.1f / %.1f\nAttributeSet MaxSpeed: %.1f\nMovement MaxSpeed: %.1f"),
        *EnemyStatsID.ToString(),
        AttributeSet->GetHealth(),
        AttributeSet->GetMaxHealth(),
        AttributeSet->GetMaxSpeed(),
        CurrentMovementSpeed
    );
}

void ACPP_EnemyCharacter::DisplayStatsOnScreen(float Duration) const
{
    if (!GEngine)
    {
        return;
    }

    const FString StatsInfo = GetDebugStatsInfo();
    const FColor DisplayColor = FColor::Green;

    // 画面に表示
    GEngine->AddOnScreenDebugMessage(
        -1,
        Duration,
        DisplayColor,
        StatsInfo
    );

    // ログにも出力
    UE_LOG(LogTemp, Display, TEXT("%s"), *StatsInfo);
}

// ============== IAbilitySystemInterface ==============
UAbilitySystemComponent* ACPP_EnemyCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

// ============== Core Accessors ==============
UCPP_PlayerAttributeSet* ACPP_EnemyCharacter::GetPlayerAttributeSet() const
{
    return AttributeSet;
}

// ============== Death Handling ==============
void ACPP_EnemyCharacter::HandleDeath()
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter::HandleDeath] %s is dying"), *GetName());
#endif

    // StateTreeを停止
    if (StateTreeComponent)
    {
        StateTreeComponent->StopLogic(TEXT("Target Destroyed"));
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter::HandleDeath] StateTree stopped"));
#endif
    }

    // 親クラスの死亡処理を呼ぶ
    Super::HandleDeath();
}