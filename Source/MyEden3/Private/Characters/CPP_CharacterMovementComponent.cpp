// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CPP_CharacterMovementComponent.h"
#include "Characters/CPP_CharacterBase.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

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

    // スライディングのデフォルト値
    MinSlidingSpeed = 400.0f;
    SlidingDeceleration = 0.98f;
    MinSlidingEndSpeed = 100.0f;
    SlidingControlStrength = 0.3f;
    SlidingCapsuleHeightRatio = 0.5f;
    GroundFrictionRate = 0.002f;
    AirResistanceRate = 0.001f;
    GroundSlidingInitialBoost = 1.2f;

    PrimaryComponentTick.bCanEverTick = true;
}

void UCPP_CharacterMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // しゃがみ設定を強制的に再適用
    GetNavAgentPropertiesRef().bCanCrouch = true;
    bCanWalkOffLedgesWhenCrouching = true;

    // 元のキャプセル高さを保存
    if (const ACharacter* Character = GetCharacterOwner())
    {
        if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            OriginalCapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
        }
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("MovementComponent BeginPlay: bCanCrouch = %s"),
        GetNavAgentPropertiesRef().bCanCrouch ? TEXT("True") : TEXT("False"));
#endif
}

void UCPP_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 直接入力状態をチェック（Blueprint入力バインディングに依存しない）
    if (bIsSliding)
    {
        const APlayerController* PC = nullptr;
        if (const ACharacter* Character = GetCharacterOwner())
        {
            PC = Cast<APlayerController>(Character->GetController());
        }

        bool bCurrentlyCrouchPressed = bCrouchInputPressed;

        // 入力マネージャーから直接しゃがみキーの状態を取得
        if (PC && PC->IsInputKeyDown(EKeys::LeftControl)) // または設定されているしゃがみキー
        {
            bCurrentlyCrouchPressed = true;
        }
        else if (PC)
        {
            bCurrentlyCrouchPressed = false;
        }

        // しゃがみキーが離された場合の検知
        if (bCrouchInputPressed && !bCurrentlyCrouchPressed)
        {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("Tick: Direct input check - crouch key released during sliding"));
#endif
            bCrouchInputPressed = false;
            StopSliding();
            return;
        }

        bCrouchInputPressed = bCurrentlyCrouchPressed;
    }

    // スライディング状態の更新
    if (bIsSliding)
    {
        UpdateSlidingMovement(DeltaTime);
    }

    // 移動中のみ方向別速度を更新（スライディング中は除く）
    if (!bIsSliding)
    {
        const FVector LastInput = GetLastInputVector();
        if (!LastInput.IsNearlyZero(0.01f))
        {
            UpdateDirectionalSpeedInternal(LastInput);
        }
    }
}

// ============== Crouch Override Functions ==============
bool UCPP_CharacterMovementComponent::CanCrouchInCurrentState() const
{
    // スライディング中はしゃがみ不可
    if (bIsSliding)
    {
        return false;
    }
    return Super::CanCrouchInCurrentState();
}

void UCPP_CharacterMovementComponent::Crouch(bool bClientSimulation)
{
    bCrouchInputPressed = true;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("Crouch called - OnGround: %s, Speed: %.2f, CanSlide: %s, bCrouchInputPressed: %s"),
        IsMovingOnGround() ? TEXT("True") : TEXT("False"),
        Velocity.Size(),
        CanStartSliding() ? TEXT("True") : TEXT("False"),
        bCrouchInputPressed ? TEXT("True") : TEXT("False"));
#endif

    // 地上で速度がある場合は即座にスライディング開始
    if (IsMovingOnGround() && CanStartSliding())
    {
        StartSliding();
        return;
    }

    // 地上で速度が足りない場合、または空中の場合は通常のしゃがみ
    if (IsMovingOnGround())
    {
        Super::Crouch(bClientSimulation);
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Regular crouch executed"));
#endif
    }
}

void UCPP_CharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("UnCrouch called - IsSliding: %s, bCrouchInputPressed before: %s"),
        bIsSliding ? TEXT("True") : TEXT("False"),
        bCrouchInputPressed ? TEXT("True") : TEXT("False"));
#endif

    bCrouchInputPressed = false;

    // スライディング中なら即座に停止（最優先）
    if (bIsSliding)
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Force stopping sliding due to crouch key release"));
#endif
        StopSliding();
        return;
    }

    // 通常のしゃがみ解除
    Super::UnCrouch(bClientSimulation);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("UnCrouch completed - bCrouchInputPressed: %s"),
        bCrouchInputPressed ? TEXT("True") : TEXT("False"));
#endif
}

void UCPP_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    const bool bJustLanded = (PreviousMovementMode == MOVE_Falling && IsMovingOnGround());

    if (bJustLanded)
    {
        // スライディング状態で着地した場合、速度を復元
        if (bIsSliding && !SlidingDirection.IsNearlyZero())
        {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            const float PreLandingSpeed = Velocity.Size();
            UE_LOG(LogTemp, Warning, TEXT("Landing while sliding - PreSpeed: %.2f"), PreLandingSpeed);
#endif

            // 着地時に水平速度が失われている場合、保存されている方向から復元
            const FVector CurrentHorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
            if (CurrentHorizontalVelocity.Size() < 100.0f) // 速度が大幅に失われた場合
            {
                // 最低限の速度で復元（MinSlidingEndSpeedの1.5倍程度）
                const float RestoredSpeed = FMath::Max(150.0f, MinSlidingEndSpeed * 1.5f);
                const FVector RestoredVelocity = SlidingDirection * RestoredSpeed;
                Velocity = FVector(RestoredVelocity.X, RestoredVelocity.Y, Velocity.Z);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
                UE_LOG(LogTemp, Warning, TEXT("Restored sliding velocity - NewSpeed: %.2f"), RestoredSpeed);
#endif
            }

            bManualVelocityControl = true; // 手動制御を再開
        }
        // 通常の着地処理
        else if (bCrouchInputPressed && CanStartSliding())
        {
            StartSliding();
        }
        else if (bCrouchInputPressed)
        {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Warning, TEXT("Landing with crouch input but insufficient speed for sliding"));
#endif
            Super::Crouch(false);
        }
    }

    // 空中移行時：スライディング中なら手動制御を停止
    if (PreviousMovementMode != MOVE_Falling && MovementMode == MOVE_Falling && bIsSliding)
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Sliding in air - disabling manual control"));
#endif
        bManualVelocityControl = false;
    }
}

// ============== Sliding Functions ==============
bool UCPP_CharacterMovementComponent::CanStartSliding() const
{
    const bool bAlreadySliding = bIsSliding;
    const bool bOnGround = IsMovingOnGround();
    const bool bCrouchPressed = bCrouchInputPressed;
    const float CurrentSpeed = Velocity.Size();
    const bool bSpeedOK = CurrentSpeed >= MinSlidingSpeed;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("CanStartSliding - AlreadySliding: %s, OnGround: %s, CrouchPressed: %s, Speed: %.2f (Min: %.2f), Result: %s"),
        bAlreadySliding ? TEXT("True") : TEXT("False"),
        bOnGround ? TEXT("True") : TEXT("False"),
        bCrouchPressed ? TEXT("True") : TEXT("False"),
        CurrentSpeed, MinSlidingSpeed,
        (!bAlreadySliding && bOnGround && bCrouchPressed && bSpeedOK) ? TEXT("True") : TEXT("False"));
#endif

    if (bAlreadySliding) return false;
    if (!bOnGround) return false;
    if (!bCrouchPressed) return false; // しゃがみキーが離されている場合は開始不可
    return bSpeedOK;
}

void UCPP_CharacterMovementComponent::StartSliding()
{
    if (!CanStartSliding())
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Error, TEXT("StartSliding called but CanStartSliding returned false!"));
#endif
        return;
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("Starting Sliding - Speed: %.2f"), Velocity.Size());
#endif

    bIsSliding = true;
    bManualVelocityControl = true;
    SlidingDirection = Velocity.GetSafeNormal();
    InitialSlidingVelocity = Velocity;
    bStartedSlidingOnGround = IsMovingOnGround();

    if (bStartedSlidingOnGround && GroundSlidingInitialBoost > 1.0f)
    {
        const float CurrentSpeed = Velocity.Size();
        const float BoostedSpeed = CurrentSpeed * GroundSlidingInitialBoost;
        Velocity = SlidingDirection * BoostedSpeed;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Ground sliding boost applied: %.2f -> %.2f (x%.2f)"),
            CurrentSpeed, BoostedSpeed, GroundSlidingInitialBoost);
#endif
    }

    SetSlidingCapsuleHeight();

    if (ACPP_CharacterBase* Character = Cast<ACPP_CharacterBase>(GetCharacterOwner()))
    {
        Character->OnSlidingStarted();
    }
}

void UCPP_CharacterMovementComponent::StopSliding()
{
    if (!bIsSliding) return;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("StopSliding - CrouchInput: %s, IsCrouching: %s"),
        bCrouchInputPressed ? TEXT("Pressed") : TEXT("Released"),
        IsCrouching() ? TEXT("True") : TEXT("False"));
#endif

    bIsSliding = false;
    bManualVelocityControl = false;
    bStartedSlidingOnGround = false;

    if (ACPP_CharacterBase* Character = Cast<ACPP_CharacterBase>(GetCharacterOwner()))
    {
        Character->OnSlidingEnded();
    }

    SlidingDirection = FVector::ZeroVector;
    InitialSlidingVelocity = FVector::ZeroVector;

    // キャプセル高さを元に戻す
    RestoreOriginalCapsuleHeight();

    // スライディング終了後は常に立ち上がる（しゃがみキーの状態に関係なく）
    if (IsCrouching())
    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Force standing up after sliding"));
#endif
        Super::UnCrouch(false);
    }

    // bCrouchInputPressedをリセット
    bCrouchInputPressed = false;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("StopSliding completed - Final IsCrouching: %s, bCrouchInputPressed: %s"),
        IsCrouching() ? TEXT("True") : TEXT("False"),
        bCrouchInputPressed ? TEXT("True") : TEXT("False"));
#endif
}

bool UCPP_CharacterMovementComponent::CanRestoreCapsuleHeight() const
{
    if (const ACharacter* Character = GetCharacterOwner())
    {
        if (const UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            const FVector Start = Character->GetActorLocation();
            const FVector End = Start + FVector(0, 0, OriginalCapsuleHalfHeight - CapsuleComp->GetUnscaledCapsuleHalfHeight());

            FHitResult HitResult;
            const float CapsuleRadius = CapsuleComp->GetUnscaledCapsuleRadius();

            return !GetWorld()->SweepSingleByChannel(
                HitResult,
                Start,
                End,
                FQuat::Identity,
                ECC_WorldStatic,
                FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleComp->GetUnscaledCapsuleHalfHeight())
            );
        }
    }
    return true;
}

void UCPP_CharacterMovementComponent::PerformMovement(float DeltaTime)
{
    if (bIsSliding)
    {
        // スライディング中の処理
        if (IsMovingOnGround() && bManualVelocityControl)
        {
            // 地上でのスライディング：手動で速度制御
            UpdateSlidingMovement(DeltaTime);

            // 現在の速度を保存
            const FVector PreMovementVelocity = Velocity;

            // 親クラスの移動処理を実行
            Super::PerformMovement(DeltaTime);

            // スライディング中なら水平速度を復元（垂直速度は物理演算に任せる）
            if (bIsSliding) // 移動処理中にスライディングが終了していないかチェック
            {
                const FVector HorizontalVelocity = FVector(PreMovementVelocity.X, PreMovementVelocity.Y, 0.0f);
                Velocity = HorizontalVelocity + FVector(0, 0, Velocity.Z);
            }
        }
        else
        {
            // 空中でのスライディング：通常の物理処理を使用
            Super::PerformMovement(DeltaTime);

            // 着地時の速度保持のため、水平速度を記録
            if (!IsMovingOnGround())
            {
                const FVector CurrentHorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
                if (!CurrentHorizontalVelocity.IsNearlyZero())
                {
                    SlidingDirection = CurrentHorizontalVelocity.GetSafeNormal();
                }
            }
        }
    }
    else
    {
        // 通常の移動処理
        Super::PerformMovement(DeltaTime);
    }
}

void UCPP_CharacterMovementComponent::UpdateSlidingMovement(float DeltaTime)
{
    if (!bIsSliding)
    {
        return;
    }

    // この時点でbCrouchInputPressedのチェックは不要（TickComponentで既にチェック済み）

    // 速度による終了条件チェック
    if (ShouldEndSliding())
    {
        StopSliding();
        return;
    }

    // スライディング物理処理
    HandleSlidingPhysics(DeltaTime);
}

void UCPP_CharacterMovementComponent::HandleSlidingPhysics(float DeltaTime)
{
    if(!bIsSliding) return;

    // 現在の水平速度を取得
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    const float CurrentSpeed = HorizontalVelocity.Size();

    // 基本減速率を計算
    float FinalDecelerationRate = SlidingDeceleration;

    // 速度に応じた動的な減速調整
    const float SpeedFactor = FMath::Clamp(CurrentSpeed / 800.0f, 0.5f, 1.2f);
    FinalDecelerationRate = FMath::Lerp(0.992f, 0.998f, SpeedFactor);

    // 地面摩擦と空気抵抗による減速
    const float FrameRate = DeltaTime * 60.0f;
    FinalDecelerationRate -= (GroundFrictionRate * FrameRate);
    FinalDecelerationRate -= (AirResistanceRate * FMath::Square(CurrentSpeed / 1000.0f) * FrameRate);

    // 地面の傾斜による影響
    FHitResult GroundHit;
    if (GetGroundInfo(GroundHit))
    {
        const FVector GroundNormal = GroundHit.Normal;
        const float SlopeAngle = FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector));
        const float SlopeInfluence = FMath::Sin(SlopeAngle) * 0.008f;

        const FVector SlopeDirection = FVector::CrossProduct(
            FVector::CrossProduct(GroundNormal, FVector::UpVector),
            GroundNormal
        ).GetSafeNormal();

        // 傾斜方向に応じた加速・減速
        if (FVector::DotProduct(SlidingDirection, SlopeDirection) > 0)
        {
            FinalDecelerationRate += SlopeInfluence; // 下り坂
        }
        else
        {
            FinalDecelerationRate -= SlopeInfluence * 0.5f; // 上り坂
        }

        // 地面に沿って滑る
        HorizontalVelocity = FVector::VectorPlaneProject(HorizontalVelocity, GroundNormal);
    }

    // 減速率をクランプ
    FinalDecelerationRate = FMath::Clamp(FinalDecelerationRate, 0.98f, 1.01f);
    const float NewSpeed = CurrentSpeed * FinalDecelerationRate;

    // 入力による方向制御
    const FVector InputDirection = GetLastInputVector();
    if (!InputDirection.IsNearlyZero() && SlidingControlStrength > 0.0f)
    {
        SlidingDirection = FMath::Lerp(
            SlidingDirection,
            InputDirection.GetSafeNormal(),
            SlidingControlStrength * DeltaTime
        ).GetSafeNormal();
    }

    // 新しい速度を適用
    const FVector NewHorizontalVelocity = SlidingDirection * NewSpeed;
    Velocity = FVector(NewHorizontalVelocity.X, NewHorizontalVelocity.Y, Velocity.Z);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Slide: %.1f->%.1f (Rate:%.3f)"), CurrentSpeed, NewSpeed, FinalDecelerationRate));
    }
#endif
}

bool UCPP_CharacterMovementComponent::GetGroundInfo(FHitResult& OutHit) const
{
    if (const ACharacter* Character = GetCharacterOwner())
    {
        const FVector Start = Character->GetActorLocation();
        const FVector End = Start - FVector(0, 0, 100.0f); // 下向きに100cm

        return GetWorld()->LineTraceSingleByChannel(
            OutHit,
            Start,
            End,
            ECC_WorldStatic,
            FCollisionQueryParams(FName(TEXT("GroundCheck")), true, Character)
        );
    }
    return false;
}

bool UCPP_CharacterMovementComponent::ShouldEndSliding() const
{
    // 速度が最低終了速度を下回った場合
    const float CurrentSpeed = Velocity.Size();
    const bool bSpeedTooLow = CurrentSpeed < MinSlidingEndSpeed;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("ShouldEndSliding - Speed: %.2f (Min: %.2f), SpeedTooLow: %s"),
        CurrentSpeed, MinSlidingEndSpeed, bSpeedTooLow ? TEXT("True") : TEXT("False"));
#endif

    return bSpeedTooLow;
}

void UCPP_CharacterMovementComponent::SetSlidingCapsuleHeight()
{
    if (ACharacter* Character = GetCharacterOwner())
    {
        if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            const float NewHalfHeight = OriginalCapsuleHalfHeight * SlidingCapsuleHeightRatio;
            CapsuleComp->SetCapsuleHalfHeight(NewHalfHeight);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Log, TEXT("Capsule height set to: %.2f (ratio: %.2f)"),
                NewHalfHeight, SlidingCapsuleHeightRatio);
#endif
        }
    }
}

void UCPP_CharacterMovementComponent::RestoreOriginalCapsuleHeight()
{
    if (ACharacter* Character = GetCharacterOwner())
    {
        if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            CapsuleComp->SetCapsuleHalfHeight(OriginalCapsuleHalfHeight);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Log, TEXT("Capsule height restored to: %.2f"), OriginalCapsuleHalfHeight);
#endif
        }
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
    // スライディング中は方向別速度更新を無効化
    if (bIsSliding)
    {
        return;
    }
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

void UCPP_CharacterMovementComponent::DebugSlidingState() const
{
    const ACharacter* Character = GetCharacterOwner();
    if (!Character) return;

    UE_LOG(LogTemp, Warning, TEXT("=== SLIDING DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Is Sliding: %s"), bIsSliding ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Warning, TEXT("Current Speed: %.2f"), Velocity.Size());
    UE_LOG(LogTemp, Warning, TEXT("Sliding Direction: %s"), *SlidingDirection.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Can Start Sliding: %s"), CanStartSliding() ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Warning, TEXT("Min Sliding Speed: %.2f"), MinSlidingSpeed);
    UE_LOG(LogTemp, Warning, TEXT("====================="));
}
#endif