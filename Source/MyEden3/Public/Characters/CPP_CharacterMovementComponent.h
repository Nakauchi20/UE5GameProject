// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CPP_CharacterMovementComponent.generated.h"

/**
 *
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

    // ============== Public Interface ==============
    UFUNCTION(BlueprintCallable, Category = "Custom Movement")
    void SetMaxSpeedWalk(float NewSpeed);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom Movement")
    float GetMaxSpeedWalk() const { return MaxSpeedWalk; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom Movement")
    FVector GetCurrentMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Custom Movement")
    void UpdateDirectionalSpeed();

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDirectionalMovement() const;
#endif

private:

    UPROPERTY(BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
    float MaxSpeedWalk = 600.0f;

    // Internal helper functions
    float CalculateDirectionalSpeedMultiplier(const FVector& InputDirection) const;
    void UpdateDirectionalSpeedInternal(const FVector& InputDirection);
    void UpdateCrouchSpeed(float Multiplier);
    void UpdateWalkSpeed(float Multiplier);

};
