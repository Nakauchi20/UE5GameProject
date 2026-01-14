// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "CPP_WeaponBase.generated.h"

// 前方宣言
class UAbilitySystemComponent;

UCLASS()
class MYEDEN3_API ACPP_WeaponBase : public AActor
{
	GENERATED_BODY()

	// === コンポーネント ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* TraceBox;

	// === モジュラー武器パーツ ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|WeaponParts", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* HandGuard;

	// HandGuardの子コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Attachments", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Scope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Attachments", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Stock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Attachments", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Barrel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Attachments", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Attachments", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Trigger;

	// ロケットシステム
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rockets", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* RocketSingle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rockets", meta = (AllowPrivateAccess = "true"))
	TArray<class USkeletalMeshComponent*> MiniRockets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rockets", meta = (AllowPrivateAccess = "true"))
	TArray<class USkeletalMeshComponent*> MicroRockets;

public:

	ACPP_WeaponBase();

	// === アクション関数（GAから呼ばれる。タグ管理はGA側で行う） ===
	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	void Fire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	void FireOngoing();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
	void OnReloadAnimationComplete();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
	void InterruptReload();

	UFUNCTION(BlueprintPure, Category = "Weapon|Components")
	USkeletalMeshComponent* GetAttachment(const FName& AttachmentName);

	// === Blueprint実装イベント ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void FireAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void FireOngoingAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void PlayReloadAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void ReloadEffect();

	// === ゲッター関数（BlueprintPure） ===
	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetMaxAmmo() const { return MaxAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetAmmo() const { return Ammo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetStockAmmo() const { return StockAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetWeaponDamage() const { return WeaponDamage; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetFireRate() const { return FireRate; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetFireRange() const { return FireRange; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetFireSpread() const { return FireSpread; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
	FORCEINLINE float GetZoomScale() const { return ZoomScale; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Info")
	FORCEINLINE FString GetWeaponName() const { return WeaponName; }

	UFUNCTION(BlueprintPure, Category = "Weapon|State")
	FORCEINLINE bool IsFullAuto() const { return bIsFullAuto; }

	UFUNCTION(BlueprintPure, Category = "Weapon|State")
	FORCEINLINE bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Weapon|State")
	FORCEINLINE bool CanFire() const { return bCanFire && !bIsReloading; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon|State")
	bool NeedsReload() const { return Ammo < MaxAmmo && (StockAmmo > 0.0f || bInfiniteStockAmmo); }

	// インライン取得関数
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE UBoxComponent* GetBox() const { return TraceBox; }
	FORCEINLINE USkeletalMeshComponent* GetHandGuard() const { return HandGuard; }

protected:

	virtual void BeginPlay() override;

	// === 内部処理関数 ===
	void SetCanFire();
	void FireEffect();
	void ConsumeAmmo();
	bool HasAmmo() const { return Ammo > 0.0f; }
	void RegenerateAmmo();
	void StartAmmoRegeneration();
	void StopAmmoRegeneration();
	void RestoreConsumedAmmo();

	// === GAS統合（非推奨、後方互換性のみ） ===
	// 注意: タグ管理はGameplayAbilityで行うべきです
	// これらの関数は古いコードとの互換性のためだけに残されています
	void AddReloadingTag();
	void RemoveReloadingTag();
	void AddFiringTag();
	void RemoveFiringTag();
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

	// === タイマーハンドル ===
	FTimerHandle FireRateTimerHandle;
	FTimerHandle AmmoRegenerationTimerHandle;

	// === 武器の状態 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	bool bCanFire;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	bool bIsReloading;

	// リロード前の弾薬量を保存（中断時の復元用）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	float AmmoBeforeReload;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	float StockAmmoBeforeReload;

public:

	// 武器情報
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Info")
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Info")
	bool bIsFullAuto;

	// 弾薬設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	float MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	float Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	float StockAmmo;

	// 弾薬自動回復設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	bool bInfiniteStockAmmo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	bool bEnableAmmoRegeneration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0.1"))
	float AmmoRegenerationInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	float AmmoRegenerationAmount = 1.0f;

	// 戦闘パラメータ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat", meta = (ClampMin = "0"))
	int BulletsPerTap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat", meta = (ClampMin = "0"))
	float WeaponDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat", meta = (ClampMin = "0.01"))
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat", meta = (ClampMin = "0"))
	float FireRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat", meta = (ClampMin = "0"))
	float FireSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ZoomScale;

	// エフェクト設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	USoundBase* WeaponSwapSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	FVector MuzzleOffset;

};