// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CPP_CombatLibrary.h"
#include "Characters/CPP_CharacterBase.h"
#include "Components/CPP_HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

void UCPP_CombatLibrary::ApplyDamage(
    AActor* TargetActor,
    float DamageAmount,
    AActor* DamageCauser)
{
    if (!TargetActor || DamageAmount <= 0.0f)
    {
        return;
    }

    bool bDamageApplied = false;

    // PlayerCharacterまたはEnemyCharacterの場合は直接ReceiveDamageを呼ぶ
    if (ACPP_CharacterBase* CharacterBase = Cast<ACPP_CharacterBase>(TargetActor))
    {
        CharacterBase->ReceiveDamage(DamageAmount, DamageCauser);
        bDamageApplied = true;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
        UE_LOG(LogTemp, Log, TEXT("ApplyDamage: %s received %.1f damage from %s"),
            *CharacterBase->GetName(), DamageAmount, *CauserName);
#endif
    }
    // HealthComponentを持っている場合
    else if (UCPP_HealthComponent* HealthComp = TargetActor->FindComponentByClass<UCPP_HealthComponent>())
    {
        HealthComp->ReceiveDamage(DamageAmount, DamageCauser);
        bDamageApplied = true;
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    if (!bDamageApplied)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: %s cannot receive damage (no Character or HealthComponent)"),
            *TargetActor->GetName());
    }
#endif
}

void UCPP_CombatLibrary::ApplyHealing(
    AActor* TargetActor,
    float HealAmount,
    AActor* Healer)
{
    if (!TargetActor || HealAmount <= 0.0f)
    {
        return;
    }

    bool bHealingApplied = false;

    // PlayerCharacterまたはEnemyCharacterの場合は直接ApplyHealingToSelfを呼ぶ
    if (ACPP_CharacterBase* CharacterBase = Cast<ACPP_CharacterBase>(TargetActor))
    {
        CharacterBase->ApplyHealingToSelf(HealAmount);
        bHealingApplied = true;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        FString HealerName = Healer ? Healer->GetName() : TEXT("Unknown");
        UE_LOG(LogTemp, Log, TEXT("ApplyHealing: %s healed %.1f HP from %s"),
            *CharacterBase->GetName(), HealAmount, *HealerName);
#endif
    }
    // HealthComponentを持っている場合
    else if (UCPP_HealthComponent* HealthComp = TargetActor->FindComponentByClass<UCPP_HealthComponent>())
    {
        HealthComp->Heal(HealAmount);
        bHealingApplied = true;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
        FString HealerName = Healer ? Healer->GetName() : TEXT("Unknown");
        UE_LOG(LogTemp, Log, TEXT("ApplyHealing: %s (HealthComponent) healed %.1f HP from %s"),
            *TargetActor->GetName(), HealAmount, *HealerName);
#endif
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    if (!bHealingApplied)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyHealing: %s cannot receive healing (no Character or HealthComponent)"),
            *TargetActor->GetName());
    }
#endif
}

void UCPP_CombatLibrary::ApplyExplosionDamage(
    UObject* WorldContextObject,
    FVector ExplosionCenter,
    float ExplosionRadius,
    float MaxDamage,
    AActor* DamageCauser,
    bool bCheckLineOfSight,
    bool bDebugDraw)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return;
    }

    // 範囲内のすべてのActorを取得
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Cast<AActor>(WorldContextObject));

    // Sphere Overlapで範囲内のActorを検索
    World->OverlapMultiByChannel(
        OverlapResults,
        ExplosionCenter,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(ExplosionRadius),
        QueryParams
    );

    // ダメージを与える
    int32 DamagedCount = 0;
    for (const FOverlapResult& Result : OverlapResults)
    {
        if (AActor* HitActor = Result.GetActor())
        {
            // PlayerCharacterまたはEnemyCharacterまたはHealthComponent
            bool bCanReceiveDamage = false;
            ACPP_CharacterBase* CharacterBase = nullptr;
            UCPP_HealthComponent* HealthComp = nullptr;

            // キャラクターかチェック
            CharacterBase = Cast<ACPP_CharacterBase>(HitActor);

            if (CharacterBase)
            {
                bCanReceiveDamage = true;
            }
            else
            {
                // HealthComponentを持っているかチェック
                HealthComp = HitActor->FindComponentByClass<UCPP_HealthComponent>();
                if (HealthComp)
                {
                    bCanReceiveDamage = true;
                }
            }

            if (bCanReceiveDamage)
            {
                // 距離を計算
                const float Distance = FVector::Dist(ExplosionCenter, HitActor->GetActorLocation());

                // 障害物チェック（オプション）
                if (bCheckLineOfSight)
                {
                    FHitResult HitResult;
                    FCollisionQueryParams TraceParams;
                    TraceParams.AddIgnoredActor(Cast<AActor>(WorldContextObject));
                    if (DamageCauser)
                    {
                        TraceParams.AddIgnoredActor(DamageCauser);
                    }
                    TraceParams.AddIgnoredActor(HitActor);

                    bool bHit = World->LineTraceSingleByChannel(
                        HitResult,
                        ExplosionCenter,
                        HitActor->GetActorLocation(),
                        ECC_Visibility,
                        TraceParams
                    );

                    // 障害物がある場合はスキップ
                    if (bHit && HitResult.GetActor() != HitActor)
                    {
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
                        UE_LOG(LogTemp, Log, TEXT("ApplyExplosionDamage: %s blocked by %s"),
                            *HitActor->GetName(), *HitResult.GetActor()->GetName());
#endif
                        continue;
                    }
                }

                // 距離に応じてダメージを減衰（線形補間）
                const float DamageRatio = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.0f, 1.0f);
                const float FinalDamage = MaxDamage * DamageRatio;

                // ダメージ適用
                if (FinalDamage > 0.0f)
                {
                    if (CharacterBase)
                    {
                        CharacterBase->ReceiveDamage(FinalDamage, DamageCauser);
                    }
                    else if (HealthComp)
                    {
                        HealthComp->ReceiveDamage(FinalDamage, DamageCauser);
                    }

                    DamagedCount++;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
                    UE_LOG(LogTemp, Log, TEXT("ApplyExplosionDamage: %s took %.1f damage (Distance: %.1f, Ratio: %.2f)"),
                        *HitActor->GetName(), FinalDamage, Distance, DamageRatio);
#endif
                }
            }
        }
    }

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("ApplyExplosionDamage: Damaged %d actors"), DamagedCount);

    // デバッグ描画
    if (bDebugDraw)
    {
        DrawDebugSphere(
            World,
            ExplosionCenter,
            ExplosionRadius,
            32,
            FColor::Red,
            false,
            2.0f,
            0,
            2.0f
        );
    }
#endif
}