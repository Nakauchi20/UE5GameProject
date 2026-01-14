#include "Weapon/CPP_WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/CPP_GameplayTags.h"


ACPP_WeaponBase::ACPP_WeaponBase()
{
	// コンポーネントの初期化
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = Mesh;

	TraceBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	TraceBox->SetupAttachment(Mesh);

	HandGuard = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandGuard"));
	HandGuard->SetupAttachment(Mesh);

	// デフォルト値の設定
	bCanFire = true;
	bIsReloading = false;
	bIsFullAuto = true;

	// 武器パラメータのデフォルト値
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
	MaxAmmo = 30.0f;
	Ammo = MaxAmmo;
	StockAmmo = 300.0f;
	WeaponDamage = 15.0f;
	FireRate = 0.5f;
	FireRange = 3000.0f;
	FireSpread = 2.5f;
	ZoomScale = 0.95f;

	// リロード前の弾薬量の初期化
	AmmoBeforeReload = 0.0f;
	StockAmmoBeforeReload = 0.0f;

	Scope = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Scope"));
	Scope->SetupAttachment(HandGuard);

	Stock = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Stock"));
	Stock->SetupAttachment(HandGuard);

	Barrel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Barrel"));
	Barrel->SetupAttachment(HandGuard);

	Mag = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mag"));
	Mag->SetupAttachment(HandGuard);

	Trigger = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(HandGuard);

	// ロケットシステム
	RocketSingle = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RocketSingle"));
	RocketSingle->SetupAttachment(HandGuard);

	// MiniRockets配列の初期化（6個）
	MiniRockets.Reserve(6);
	for (int32 i = 1; i <= 6; i++)
	{
		FString ComponentName = FString::Printf(TEXT("MiniRocket_%d"), i);
		USkeletalMeshComponent* MiniRocket = CreateDefaultSubobject<USkeletalMeshComponent>(*ComponentName);
		MiniRocket->SetupAttachment(HandGuard);
		MiniRockets.Add(MiniRocket);
	}

	// MicroRockets配列の初期化（5個）
	MicroRockets.Reserve(5);
	for (int32 i = 1; i <= 5; i++)
	{
		FString ComponentName = FString::Printf(TEXT("MicroRocket_%d"), i);
		USkeletalMeshComponent* MicroRocket = CreateDefaultSubobject<USkeletalMeshComponent>(*ComponentName);
		MicroRocket->SetupAttachment(HandGuard);
		MicroRockets.Add(MicroRocket);
	}
}

void ACPP_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 弾薬自動回復が有効なら開始
	if (bEnableAmmoRegeneration && Ammo < MaxAmmo && !bIsReloading)
	{
		StartAmmoRegeneration();
	}
}

void ACPP_WeaponBase::SetCanFire()
{
	bCanFire = true;
	// タグ管理はGameplayAbilityで行う（アビリティ終了時に自動削除される）
}

// ============== アクション関数（タグ管理なし） ==============
// 注意: これらの関数内ではタグの付与/削除を行いません
// タグ管理はGameplayAbilityのActivation Owned Tagsで自動的に行われます

void ACPP_WeaponBase::Fire()
{
	if (!CanFire()) return;

	if (HasAmmo())
	{
		FireAction();
		FireEffect();
		ConsumeAmmo();

		bCanFire = false;
		GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &ACPP_WeaponBase::SetCanFire, FireRate, false);

		// 弾薬を消費したら自動回復を開始
		if (bEnableAmmoRegeneration && !bIsReloading)
		{
			StartAmmoRegeneration();
		}
	}
	else
	{
		// 弾切れの場合は自動的にリロードを試みる
		Reload();
	}
}

void ACPP_WeaponBase::FireOngoing()
{
	if (!CanFire()) return;

	if (HasAmmo())
	{
		FireOngoingAction();
		FireEffect();
		ConsumeAmmo();

		bCanFire = false;
		GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &ACPP_WeaponBase::SetCanFire, FireRate, false);

		// 弾薬を消費したら自動回復を開始
		if (bEnableAmmoRegeneration && !bIsReloading)
		{
			StartAmmoRegeneration();
		}
	}
	else
	{
		// 弾切れの場合は自動的にリロードを試みる
		Reload();
	}
}

void ACPP_WeaponBase::Reload()
{
	if (!bIsReloading && NeedsReload())
	{
		// リロード前の弾薬量を保存（中断時の復元用）
		AmmoBeforeReload = Ammo;
		StockAmmoBeforeReload = StockAmmo;

		bIsReloading = true;

		// タグ管理はGameplayAbilityで行う（Activation Owned Tagsで自動付与）

		// リロード中は自動回復を停止
		StopAmmoRegeneration();

		// Blueprintでアニメーション再生
		PlayReloadAnimation();
	}
}

void ACPP_WeaponBase::ConsumeAmmo()
{
	Ammo = FMath::Max(0.0f, Ammo - 1.0f);
}

void ACPP_WeaponBase::FireEffect()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void ACPP_WeaponBase::OnReloadAnimationComplete()
{
	// リロードが中断されていた場合は何もしない
	if (!bIsReloading)
	{
		return;
	}

	// 無限弾薬モードの場合
	if (bInfiniteStockAmmo)
	{
		Ammo = MaxAmmo;
	}
	else
	{
		const float AmmoNeeded = MaxAmmo - Ammo;

		if (StockAmmo >= AmmoNeeded)
		{
			StockAmmo -= AmmoNeeded;
			Ammo = MaxAmmo;
		}
		else
		{
			Ammo += StockAmmo;
			StockAmmo = 0.0f;
		}
	}

	ReloadEffect();
	bIsReloading = false;

	//GameplayAbilityにリロード完了を通知
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (ASC)
	{
		FGameplayEventData EventData;
		EventData.Instigator = ASC->GetOwner();
		EventData.Target = ASC->GetOwner();

		ASC->HandleGameplayEvent(
			FCPP_GameplayTags::Get().Event_Weapon_ReloadComplete,
			&EventData
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Weapon] Failed to get ASC!"));
	}

	// リロード完了後、最大弾数になったので自動回復を停止
	StopAmmoRegeneration();
}

void ACPP_WeaponBase::InterruptReload()
{
	if (bIsReloading)
	{
		// 消費した弾薬を復元
		RestoreConsumedAmmo();

		bIsReloading = false;

		// リロードキャンセル後、弾薬が減っていれば自動回復を再開
		if (bEnableAmmoRegeneration && Ammo < MaxAmmo)
		{
			StartAmmoRegeneration();
		}
	}
}

void ACPP_WeaponBase::RestoreConsumedAmmo()
{
	Ammo = AmmoBeforeReload;
	StockAmmo = StockAmmoBeforeReload;
}

void ACPP_WeaponBase::RegenerateAmmo()
{
	// リロード中は回復しない
	if (bIsReloading)
	{
		StopAmmoRegeneration();
		return;
	}

	// 弾薬を回復
	float NewAmmo = FMath::Min(Ammo + AmmoRegenerationAmount, MaxAmmo);

	// 無限弾薬モードでない場合はStockAmmoから消費
	if (!bInfiniteStockAmmo && StockAmmo > 0)
	{
		float AmmoToRegenerate = NewAmmo - Ammo;
		if (StockAmmo >= AmmoToRegenerate)
		{
			StockAmmo -= AmmoToRegenerate;
			Ammo = NewAmmo;
		}
		else
		{
			Ammo += StockAmmo;
			StockAmmo = 0.0f;
		}
	}
	else if (bInfiniteStockAmmo)
	{
		Ammo = NewAmmo;
	}

	// 最大弾数に達したら自動回復を停止
	if (Ammo >= MaxAmmo)
	{
		Ammo = MaxAmmo;
		StopAmmoRegeneration();
	}
}

void ACPP_WeaponBase::StartAmmoRegeneration()
{
	// 既にタイマーが動いている場合は何もしない
	if (GetWorldTimerManager().IsTimerActive(AmmoRegenerationTimerHandle))
	{
		return;
	}

	// 自動回復が有効で、弾薬が最大でない場合のみ開始
	if (bEnableAmmoRegeneration && Ammo < MaxAmmo)
	{
		GetWorldTimerManager().SetTimer(
			AmmoRegenerationTimerHandle,
			this,
			&ACPP_WeaponBase::RegenerateAmmo,
			AmmoRegenerationInterval,
			true  // ループする
		);
	}
}

void ACPP_WeaponBase::StopAmmoRegeneration()
{
	if (GetWorldTimerManager().IsTimerActive(AmmoRegenerationTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(AmmoRegenerationTimerHandle);
	}
}

USkeletalMeshComponent* ACPP_WeaponBase::GetAttachment(const FName& AttachmentName)
{
	if (AttachmentName == "Scope") return Scope;
	if (AttachmentName == "Stock") return Stock;
	if (AttachmentName == "Barrel") return Barrel;
	if (AttachmentName == "Mag") return Mag;
	if (AttachmentName == "Trigger") return Trigger;
	if (AttachmentName == "RocketSingle") return RocketSingle;

	// MiniRocket_1 ~ MiniRocket_6 の形式をチェック
	FString NameStr = AttachmentName.ToString();
	if (NameStr.StartsWith("MiniRocket_"))
	{
		FString NumberStr = NameStr.RightChop(11);
		int32 Index = FCString::Atoi(*NumberStr) - 1;
		if (MiniRockets.IsValidIndex(Index))
		{
			return MiniRockets[Index];
		}
	}

	// MicroRocket_1 ~ MicroRocket_5 の形式をチェック
	if (NameStr.StartsWith("MicroRocket_"))
	{
		FString NumberStr = NameStr.RightChop(12);
		int32 Index = FCString::Atoi(*NumberStr) - 1;
		if (MicroRockets.IsValidIndex(Index))
		{
			return MicroRockets[Index];
		}
	}

	return nullptr;
}

// ============== GAS統合実装（非推奨、後方互換性のみ） ==============
// 警告: これらの関数は古いコードとの互換性のためだけに残されています
// 新しいコードではGameplayAbilityのActivation Owned Tagsを使用してください

UAbilitySystemComponent* ACPP_WeaponBase::GetOwnerAbilitySystemComponent() const
{
	AActor* OwnerActor = GetOwner();

	// デバッグ用ログ追加
	UE_LOG(LogTemp, Error, TEXT("[Weapon] GetOwner() = %s"),
		OwnerActor ? *OwnerActor->GetName() : TEXT("NULL"));

	if (!OwnerActor)
	{
		return nullptr;
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor))
	{
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		UE_LOG(LogTemp, Error, TEXT("[Weapon] ASC from interface = %s"),
			ASC ? *ASC->GetName() : TEXT("NULL"));
		return ASC;
	}

	return nullptr;
}

void ACPP_WeaponBase::AddReloadingTag()
{
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (ASC)
	{
		const FGameplayTag& ReloadingTag = FCPP_GameplayTags::Get().State_Weapon_Reloading;
		ASC->AddLooseGameplayTag(ReloadingTag);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
		UE_LOG(LogTemp, Warning, TEXT("[Weapon GAS] DEPRECATED: Manual tag addition. Use Activation Owned Tags instead."));
#endif
	}
}

void ACPP_WeaponBase::RemoveReloadingTag()
{
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (ASC)
	{
		const FGameplayTag& ReloadingTag = FCPP_GameplayTags::Get().State_Weapon_Reloading;
		ASC->RemoveLooseGameplayTag(ReloadingTag);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
		UE_LOG(LogTemp, Warning, TEXT("[Weapon GAS] DEPRECATED: Manual tag removal. Use Activation Owned Tags instead."));
#endif
	}
}

void ACPP_WeaponBase::AddFiringTag()
{
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (ASC)
	{
		const FGameplayTag& FiringTag = FCPP_GameplayTags::Get().State_Weapon_Firing;
		ASC->AddLooseGameplayTag(FiringTag);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
		UE_LOG(LogTemp, Warning, TEXT("[Weapon GAS] DEPRECATED: Manual tag addition. Use Activation Owned Tags instead."));
#endif
	}
}

void ACPP_WeaponBase::RemoveFiringTag()
{
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	if (ASC)
	{
		const FGameplayTag& FiringTag = FCPP_GameplayTags::Get().State_Weapon_Firing;
		ASC->RemoveLooseGameplayTag(FiringTag);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
		UE_LOG(LogTemp, Warning, TEXT("[Weapon GAS] DEPRECATED: Manual tag removal. Use Activation Owned Tags instead."));
#endif
	}
}