// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CPP_CharacterMovementComponent.generated.h"

// 前方宣言
class ACPP_PlayerCharacter;
class ACPP_CharacterBase;

/**
 *カスタム移動コンポーネント - 方向別速度制御とスライディング機能を提供
 */
UCLASS()
class MYEDEN3_API UCPP_CharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UCPP_CharacterMovementComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement overrides
    virtual bool CanCrouchInCurrentState() const override;
    virtual void Crouch(bool bClientSimulation = false) override;
    virtual void UnCrouch(bool bClientSimulation = false) override;
    virtual void PerformMovement(float DeltaTime) override;
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

public:
    // ============== Speed Configuration ==============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float ForwardSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float BackwardSpeedMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float SideSpeedMultiplier = 0.9f;

    UPROPERTY(EditAnywhere, Category = "Custom Movement")
    float DirectionThreshold = 0.95f;

    // ============== Sliding Configuration ==============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float MinSlidingSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "50.0", ClampMax = "300.0"))
    float MinSlidingEndSpeed = 100.0f;
    // スライディング中の減速率（0.0-1.0、1.0で減速なし）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float SlidingDeceleration = 0.98f;
    // スライディング中の方向制御強度（0.0で制御不能、1.0で通常通り）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SlidingControlStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "0.3", ClampMax = "0.8"))
    float SlidingCapsuleHeightRatio = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "0.0", ClampMax = "0.01"))
    float GroundFrictionRate = 0.002f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding", meta = (ClampMin = "0.0", ClampMax = "0.01"))
    float AirResistanceRate = 0.001f;

    UPROPERTY(EditAnywhere, Category = "Sliding")
    bool bEnableSlidingBoost = false;

    // ============== Public Interface ==============
    UFUNCTION(BlueprintCallable, Category = "Custom Movement")
    void SetMaxSpeedWalk(float NewSpeed);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom Movement")
    float GetMaxSpeedWalk() const { return MaxSpeedWalk; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom Movement")
    FVector GetCurrentMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Custom Movement")
    void UpdateDirectionalSpeed();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
    bool CanStandUp() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ForceEndSliding();

    // ============== Sliding Interface ==============
    UFUNCTION(BlueprintCallable, Category = "Sliding")
    bool TryStartSliding();

    UFUNCTION(BlueprintCallable, Category = "Sliding")
    bool CanStartSliding() const;

    UFUNCTION(BlueprintCallable, Category = "Sliding")
    void StartSliding();

    UFUNCTION(BlueprintCallable, Category = "Sliding")
    void StopSliding();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sliding")
    bool IsSliding() const { return bIsSliding; }

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void RequestUnCrouch();

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDirectionalMovement() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugSlidingState() const;
#endif

private:

    UPROPERTY(BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
    float MaxSpeedWalk = 600.0f;

    // ============== Sliding State ==============
    UPROPERTY(BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
    bool bIsSliding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
    FVector SlidingDirection = FVector::ZeroVector;

    UPROPERTY()
    bool bManualVelocityControl = false;

    UPROPERTY()
    FVector InitialSlidingVelocity = FVector::ZeroVector;

    UPROPERTY()
    bool bCrouchInputPressed = false;

    float OriginalCapsuleHalfHeight = 0.0f;

    // Internal helper functions
    float CalculateDirectionalSpeedMultiplier(const FVector& InputDirection) const;
    void UpdateDirectionalSpeedInternal(const FVector& InputDirection);
    void UpdateCrouchSpeed(float Multiplier);
    void UpdateWalkSpeed(float Multiplier);

    // Sliding internal functions
    void UpdateSlidingMovement(float DeltaTime);
    void HandleSlidingPhysics(float DeltaTime);
    bool ShouldEndSliding() const;
    void SetSlidingCapsuleHeight();
    void RestoreOriginalCapsuleHeight();
    bool CanRestoreCapsuleHeight() const;
    bool GetGroundInfo(FHitResult& OutHit) const;
    void HandleCrouchPressed();
    void HandleCrouchReleased();
    void HandleLanding();
    void HandleLeavingGround();
    void RestoreSlidingVelocity();
    void EndSlidingTransition(bool bIsAirborne);
    void NotifyCharacterSlidingEnded();
    void HandleStandUpTransition();
    void SetCapsuleHeightForCrouch();
};