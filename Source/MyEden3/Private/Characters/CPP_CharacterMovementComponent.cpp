// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CPP_CharacterMovementComponent.h"
#include "Characters/CPP_CharacterBase.h"
#include "GameFramework/Character.h"

UCPP_CharacterMovementComponent::UCPP_CharacterMovementComponent()
{
    // デフォルト値設定
    MaxSpeedWalk = 600.0f;
    MaxWalkSpeed = MaxSpeedWalk;
    GetNavAgentPropertiesRef().bCanCrouch = true;
    bCanWalkOffLedgesWhenCrouching = true;
    MaxWalkSpeedCrouched = 200.0f;
    SetCrouchedHalfHeight(40.0f);

    // 方向別速度の初期値設定
    ForwardSpeedMultiplier = 1.0f;
    BackwardSpeedMultiplier = 0.8f;
    SideSpeedMultiplier = 0.9f;
    DirectionThreshold = 0.95f;

    PrimaryComponentTick.bCanEverTick = true;
    //PrimaryComponentTick.TickInterval = 0.1f; // 10FPS instead of every frame
}

void UCPP_CharacterMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // しゃがみ設定を強制的に再適用
    GetNavAgentPropertiesRef().bCanCrouch = true;
    bCanWalkOffLedgesWhenCrouching = true;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("MovementComponent BeginPlay: bCanCrouch = %s"),
        GetNavAgentPropertiesRef().bCanCrouch ? TEXT("True") : TEXT("False"));
#endif
}

void UCPP_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 移動中のみ方向別速度を更新
    const FVector LastInput = GetLastInputVector();
    if (!LastInput.IsNearlyZero(0.01f)) // より厳密な閾値
    {
        UpdateDirectionalSpeedInternal(LastInput);
    }
}

FVector UCPP_CharacterMovementComponent::GetCurrentMovementDirection() const
{
    // 最後の入力ベクトルを正規化して返す
    const FVector InputVector = GetLastInputVector();
    return InputVector.IsNearlyZero() ? FVector::ZeroVector : InputVector.GetSafeNormal();
}


float UCPP_CharacterMovementComponent::CalculateDirectionalSpeedMultiplier(const FVector& InputDirection) const
{
    if (InputDirection.IsNearlyZero())
    {
        return ForwardSpeedMultiplier; // 入力がない場合は前方向速度をデフォルトとして使用
    }

    const ACharacter* Character = GetCharacterOwner();
    if (!Character)
    {
        return ForwardSpeedMultiplier;
    }

    const FVector CharacterForward = Character->GetActorForwardVector();
    const float ForwardDot = FVector::DotProduct(InputDirection.GetSafeNormal(), CharacterForward);

    // シンプルな判定ロジック
    if (ForwardDot > DirectionThreshold)
    {
        return ForwardSpeedMultiplier;
    }
    else if (ForwardDot < -DirectionThreshold)
    {
        return BackwardSpeedMultiplier;
    }
    else
    {
        // 斜め移動の考慮
        return FMath::Abs(ForwardDot) > 0.1f
            ? (ForwardDot > 0 ? ForwardSpeedMultiplier : BackwardSpeedMultiplier)
            : SideSpeedMultiplier;
    }
}

void UCPP_CharacterMovementComponent::SetMaxSpeedWalk(float NewSpeed)
{
    const float ClampedSpeed = FMath::Max(NewSpeed, 0.0f);
    if (FMath::IsNearlyEqual(MaxSpeedWalk, ClampedSpeed, 0.1f))
    {
        return; // 変更がなければ早期リターン
    }

    MaxSpeedWalk = ClampedSpeed;
    UpdateDirectionalSpeed();

    #if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Max Speed updated to: %.2f"), MaxSpeedWalk);
    #endif
}

void UCPP_CharacterMovementComponent::UpdateDirectionalSpeed()
{
    const FVector CurrentInput = GetCurrentMovementDirection();
    UpdateDirectionalSpeedInternal(CurrentInput);
}

void UCPP_CharacterMovementComponent::UpdateDirectionalSpeedInternal(const FVector& InputDirection)
{
    const float SpeedMultiplier = CalculateDirectionalSpeedMultiplier(InputDirection);

    if (IsCrouching())
    {
        UpdateCrouchSpeed(SpeedMultiplier);
    }
    else
    {
        UpdateWalkSpeed(SpeedMultiplier);
    }
}

void UCPP_CharacterMovementComponent::UpdateCrouchSpeed(float Multiplier)
{
    float GASMaxSpeedCrouch = 200.0f; // デフォルト値

    if (const ACharacter* Character = GetCharacterOwner())
    {
        if (const ACPP_CharacterBase* BaseCharacter = Cast<ACPP_CharacterBase>(Character))
        {
            const float GASValue = BaseCharacter->GetMaxSpeedCrouch();
            if (GASValue > 0.0f)
            {
                GASMaxSpeedCrouch = GASValue;
            }
        }
    }

    const float NewCrouchSpeed = GASMaxSpeedCrouch * Multiplier;
    if (!FMath::IsNearlyEqual(MaxWalkSpeedCrouched, NewCrouchSpeed, 0.1f))
    {
        MaxWalkSpeedCrouched = NewCrouchSpeed;
    }
}

void UCPP_CharacterMovementComponent::UpdateWalkSpeed(float Multiplier)
{
    const float NewWalkSpeed = MaxSpeedWalk * Multiplier;
    if (!FMath::IsNearlyEqual(MaxWalkSpeed, NewWalkSpeed, 0.1f))
    {
        MaxWalkSpeed = NewWalkSpeed;
    }
}


#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
void UCPP_CharacterMovementComponent::DebugDirectionalMovement() const
{
    const ACharacter* Character = GetCharacterOwner();
    if (!Character) return;

    const FVector CurrentInput = GetCurrentMovementDirection();
    const FVector CurrentVelocity = Character->GetVelocity();
    const float SpeedMultiplier = CalculateDirectionalSpeedMultiplier(CurrentInput);

    UE_LOG(LogTemp, Warning, TEXT("=== DIRECTIONAL MOVEMENT DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Max Speed: %.2f"), MaxSpeedWalk);
    UE_LOG(LogTemp, Warning, TEXT("Current Max Walk Speed: %.2f"), MaxWalkSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Current Max Walk Speed Crouched: %.2f"), MaxWalkSpeedCrouched);
    UE_LOG(LogTemp, Warning, TEXT("Current Input Direction: %s"), *CurrentInput.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Speed Multiplier: %.2f"), SpeedMultiplier);
    UE_LOG(LogTemp, Warning, TEXT("Current Speed: %.2f"), CurrentVelocity.Size());
    UE_LOG(LogTemp, Warning, TEXT("Is Crouching: %s"), IsCrouching() ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Warning, TEXT("Movement Mode: %d"), (int32)MovementMode);
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}
#endif