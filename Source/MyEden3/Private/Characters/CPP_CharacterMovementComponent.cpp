// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CPP_CharacterMovementComponent.h"
#include "Characters/CPP_PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/CPP_GameplayTags.h"

UCPP_CharacterMovementComponent::UCPP_CharacterMovementComponent()
{
	// デフォルト値設定
	MaxSpeedWalk = 600.0f;
	MaxWalkSpeed = MaxSpeedWalk;
	GetNavAgentPropertiesRef().bCanCrouch = true;
	bCanWalkOffLedgesWhenCrouching = true;
	MaxWalkSpeedCrouched = 200.0f;
	SetCrouchedHalfHeight(40.0f);
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
}

void UCPP_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
	// 基本的な条件チェック
	if (!CharacterOwner || !CharacterOwner->GetRootComponent())
	{
		return false;
	}

	// Unrealの標準条件を使用（移動中でもしゃがみ可能）
	return Super::CanCrouchInCurrentState();
}

void UCPP_CharacterMovementComponent::Crouch(bool bClientSimulation)
{
	bCrouchInputPressed = true;
	HandleCrouchPressed();
}

void UCPP_CharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	bCrouchInputPressed = false;
	HandleCrouchReleased();
}

void UCPP_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	const bool bJustLanded = (PreviousMovementMode == MOVE_Falling && IsMovingOnGround());
	const bool bJustLeftGround = (PreviousMovementMode != MOVE_Falling && MovementMode == MOVE_Falling);

	// ★ デバッグログ追加
	UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] OnMovementModeChanged - Previous: %d, Current: %d"),
		(int32)PreviousMovementMode, (int32)MovementMode);
	UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] bJustLeftGround: %s, bJustLanded: %s"),
		bJustLeftGround ? TEXT("TRUE") : TEXT("FALSE"),
		bJustLanded ? TEXT("TRUE") : TEXT("FALSE"));

	// ★ InAirタグの管理（PlayerCharacterのみ）
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetCharacterOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] ASI found: %s"), *GetCharacterOwner()->GetName());

		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] ASC found: %s"), *ASC->GetName());

			const FGameplayTag& InAirTag = FCPP_GameplayTags::Get().State_Movement_InAir;
			UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] InAirTag: %s"), *InAirTag.ToString());

			if (bJustLeftGround)
			{
				ASC->AddLooseGameplayTag(InAirTag);
				UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] ★ InAir Tag ADDED"));
			}
			else if (bJustLanded)
			{
				ASC->RemoveLooseGameplayTag(InAirTag);
				UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] ★ InAir Tag REMOVED"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MovementComponent] ASC is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MovementComponent] ASI Cast failed for: %s"),
			GetCharacterOwner() ? *GetCharacterOwner()->GetName() : TEXT("NULL"));
	}

	if (bJustLanded)
	{
		HandleLanding();
	}
	else if (bJustLeftGround)
	{
		HandleLeavingGround();
	}
}

void UCPP_CharacterMovementComponent::HandleLanding()
{
	// スライディング中の着地
	if (bIsSliding)
	{
		// 速度を維持してスライディング継続
		RestoreSlidingVelocity();
		return;
	}

	// しゃがみキーが押されている場合
	if (bCrouchInputPressed)
	{
		const float CurrentSpeed = Velocity.Size();
		if (CurrentSpeed >= MinSlidingSpeed)
		{
			// 高速着地 → スライディング開始
			if (IsCrouching())
			{
				Super::UnCrouch(false);  // 一旦しゃがみ解除
			}
			StartSliding();
		}
		else
		{
			// 低速着地 → しゃがみ維持
			if (!IsCrouching())
			{
				Super::Crouch(false);
			}
		}
	}
}

void UCPP_CharacterMovementComponent::HandleLeavingGround()
{
	if (bIsSliding)
	{
		// スライディング中断 → 空中しゃがみへ移行
		EndSlidingTransition(true);  // 空中移行フラグ付き
	}
}

// ============== Sliding Functions ==============
bool UCPP_CharacterMovementComponent::TryStartSliding()
{
	if (!CanStartSliding())
	{
		return false;
	}

	StartSliding();
	return true;
}

bool UCPP_CharacterMovementComponent::CanStartSliding() const
{
	const bool bAlreadySliding = bIsSliding;
	const bool bOnGround = IsMovingOnGround();  // 地上でのみスライディング可能
	const bool bCrouchPressed = bCrouchInputPressed;
	const float CurrentSpeed = Velocity.Size();
	const bool bSpeedOK = CurrentSpeed >= MinSlidingSpeed;

	// 地上でのみ、かつ十分な速度がある場合のみスライディング開始可能
	if (bAlreadySliding) return false;
	if (!bOnGround) return false;       // 空中では絶対にスライディング不可
	if (!bCrouchPressed) return false;
	return bSpeedOK;
}

void UCPP_CharacterMovementComponent::StartSliding()
{
	if (!CanStartSliding())
	{
		return;
	}

	bIsSliding = true;
	bManualVelocityControl = true;// 手動速度制御ON
	SlidingDirection = Velocity.GetSafeNormal();// 滑る方向を記録
	InitialSlidingVelocity = Velocity;// 初期速度を保存

	//スライディング開始時に強制的にしゃがみ状態にする
	if (!IsCrouching())
	{
		Super::Crouch(false);
	}

	SetSlidingCapsuleHeight();// キャプセル高さを50%に縮小


	// 初速ブースト
	if (bEnableSlidingBoost)
	{
		const float SlidingBoostMultiplier = 1.1f;  // 10%ブースト
		Velocity *= SlidingBoostMultiplier;
		InitialSlidingVelocity = Velocity;  // ブースト後の速度を保存
	}

	// PlayerCharacterの場合のみスライディングタグとイベントを適用
	if (ACPP_PlayerCharacter* PlayerCharacter = Cast<ACPP_PlayerCharacter>(GetCharacterOwner()))
	{
		// Slidingタグを追加
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerCharacter))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				const FGameplayTag& SlidingTag = FCPP_GameplayTags::Get().State_Movement_Sliding;
				ASC->AddLooseGameplayTag(SlidingTag);
				UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] Sliding Tag ADDED"));
			}
		}

		PlayerCharacter->OnSlidingStarted();// Blueprintイベント発火
	}
}

void UCPP_CharacterMovementComponent::StopSliding()
{
	if (!bIsSliding)
	{
		return;
	}

	// スライディング状態をクリア
	bIsSliding = false;
	bManualVelocityControl = false;
	SlidingDirection = FVector::ZeroVector;
	InitialSlidingVelocity = FVector::ZeroVector;

	// PlayerCharacterの場合のみタグ削除と通知
	if (ACPP_PlayerCharacter* PlayerCharacter = Cast<ACPP_PlayerCharacter>(GetCharacterOwner()))
	{
		// Slidingタグを削除
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerCharacter))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				const FGameplayTag& SlidingTag = FCPP_GameplayTags::Get().State_Movement_Sliding;
				ASC->RemoveLooseGameplayTag(SlidingTag);
				UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] Sliding Tag REMOVED"));
			}
		}

		PlayerCharacter->OnSlidingEnded();
	}

	// 立ち上がり処理
	if (!bCrouchInputPressed)
	{
		if (CanStandUp())
		{
			RestoreOriginalCapsuleHeight();

			if (IsCrouching())
			{
				Super::UnCrouch(false);
			}
		}
		else
		{
			// 立ち上がれない場合はしゃがみ高さに設定
			SetCapsuleHeightForCrouch();
			if (!IsCrouching())
			{
				Super::Crouch(false);
			}
		}
	}
	else
	{
		//しゃがみキーが押されている場合は通常のしゃがみ状態を維持
		SetCapsuleHeightForCrouch();
		if (!IsCrouching())
		{
			Super::Crouch(false);
		}
	}
}


void UCPP_CharacterMovementComponent::HandleCrouchPressed()
{
	// 既にスライディング中なら何もしない
	if (bIsSliding) return;

	// 1. 地上/空中判定
	if (!IsMovingOnGround())
	{
		// 空中 → 常にしゃがみ（親クラスの処理を呼び出し）
		Super::Crouch(false);
		return;
	}

	// 2. 地上 → 速度チェック
	if (Velocity.Size() >= MinSlidingSpeed)
	{
		StartSliding();
	}
	else
	{
		Super::Crouch(false);
	}
}

void UCPP_CharacterMovementComponent::HandleCrouchReleased()
{
	// スライディング中の場合
	if (bIsSliding)
	{
		// スライディング停止
		StopSliding();
	}
	// しゃがみ中の場合
	else if (IsCrouching())
	{
		// 立ち上がり可能チェック
		if (!CanStandUp())
		{
			return;
		}
		// しゃがみ解除
		Super::UnCrouch(false);
	}
}


bool UCPP_CharacterMovementComponent::CanStandUp() const
{
	const ACharacter* Character = GetCharacterOwner();
	if (!Character)
	{
		return true;
	}

	const UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
	if (!CapsuleComp)
	{
		return true;
	}

	// 現在のキャプセル高さと目標高さを取得
	const float CurrentHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const float TargetHalfHeight = OriginalCapsuleHalfHeight;

	// 既に立ち上がっている場合
	if (FMath::IsNearlyEqual(CurrentHalfHeight, TargetHalfHeight, 2.0f))
	{
		return true;
	}

	// 頭上の障害物チェック
	const FVector Start = Character->GetActorLocation();
	const float HeightDifference = TargetHalfHeight - CurrentHalfHeight + 10.0f; // 少し余裕を持たせる
	const FVector End = Start + FVector(0, 0, HeightDifference);

	FHitResult HitResult;
	const float CapsuleRadius = CapsuleComp->GetUnscaledCapsuleRadius() * 0.9f; // 少し小さくして判定

	FCollisionQueryParams QueryParams(FName(TEXT("StandUpCheck")), false, Character);
	QueryParams.bTraceComplex = false;

	const bool bBlocked = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeCapsule(CapsuleRadius, CurrentHalfHeight),
		QueryParams
	);
	return !bBlocked;
}

void UCPP_CharacterMovementComponent::RequestUnCrouch()
{
	if (CanStandUp())
	{
		UnCrouch(false);
	}
}


void UCPP_CharacterMovementComponent::RestoreSlidingVelocity()
{
	// スライディング中の速度復元（既存のコードから抽出）
	if (!SlidingDirection.IsNearlyZero())
	{
		const FVector CurrentHorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
		if (CurrentHorizontalVelocity.Size() < 100.0f)
		{
			const float RestoredSpeed = FMath::Max(150.0f, MinSlidingEndSpeed * 1.5f);
			const FVector RestoredVelocity = SlidingDirection * RestoredSpeed;
			Velocity = FVector(RestoredVelocity.X, RestoredVelocity.Y, Velocity.Z);
		}
		bManualVelocityControl = true;
	}
}

void UCPP_CharacterMovementComponent::EndSlidingTransition(bool bIsAirborne)
{
	// スライディング状態をクリア
	bIsSliding = false;
	bManualVelocityControl = false;

	// PlayerCharacterの場合のみSlidingタグを削除
	if (ACPP_PlayerCharacter* PlayerCharacter = Cast<ACPP_PlayerCharacter>(GetCharacterOwner()))
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerCharacter))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				const FGameplayTag& SlidingTag = FCPP_GameplayTags::Get().State_Movement_Sliding;
				ASC->RemoveLooseGameplayTag(SlidingTag);
				UE_LOG(LogTemp, Warning, TEXT("[MovementComponent] Sliding Tag REMOVED (Air Transition)"));
			}
		}
	}

	// しゃがみキーが押されていない場合のみキャプセル高さを復元
	if (!bCrouchInputPressed)
	{
		RestoreOriginalCapsuleHeight();
	}
	else
	{
		// しゃがみキーが押されている場合はしゃがみ高さを維持
		SetCapsuleHeightForCrouch();
	}

	// 空中移行の場合、しゃがみキーが押されていればしゃがみ状態にする
	if (bIsAirborne && bCrouchInputPressed && !IsCrouching())
	{
		Super::Crouch(false);
	}
	else if (!bIsAirborne && !bCrouchInputPressed && IsCrouching())
	{
		// 地上でしゃがみキーが押されていない場合はしゃがみ解除
		Super::UnCrouch(false);
	}

	// キャラクターに通知
	NotifyCharacterSlidingEnded();
}

void UCPP_CharacterMovementComponent::NotifyCharacterSlidingEnded()
{
	// PlayerCharacterの場合のみイベント通知
	if (ACPP_PlayerCharacter* PlayerCharacter = Cast<ACPP_PlayerCharacter>(GetCharacterOwner()))
	{
		PlayerCharacter->OnSlidingEnded();
	}
}

void UCPP_CharacterMovementComponent::SetCapsuleHeightForCrouch()
{
	if (ACharacter* Character = GetCharacterOwner())
	{
		if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
		{
			// しゃがみ用の高さに設定（GetCrouchedHalfHeightの値を使用）
			const float CrouchHeight = GetCrouchedHalfHeight();
			CapsuleComp->SetCapsuleHalfHeight(CrouchHeight);
		}
	}
}



void UCPP_CharacterMovementComponent::ForceEndSliding()
{
	if (bIsSliding)
	{
		bCrouchInputPressed = false; // 強制的にクリア
		StopSliding();
	}
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
	if (!bIsSliding) return;

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

	// スライディング中でもしゃがみ状態として判定される
	if (IsCrouching() || bIsSliding) // スライディング中も考慮
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
		// PlayerCharacterまたはEnemyCharacterからGAS値を取得
		if (const ACPP_CharacterBase* CharacterBase = Cast<ACPP_CharacterBase>(Character))
		{
			const float GASValue = CharacterBase->GetMaxSpeedCrouch();
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