// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/CPP_GA_WeaponBase.h"
#include "Characters/CPP_PlayerCharacter.h"
#include "Weapon/CPP_WeaponBase.h"
#include "AbilitySystemComponent.h"

UCPP_GA_WeaponBase::UCPP_GA_WeaponBase()
{
	// アビリティの基本設定
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

ACPP_WeaponBase* UCPP_GA_WeaponBase::GetCurrentWeapon() const
{
	ACPP_PlayerCharacter* PlayerChar = GetPlayerCharacter();
	if (!PlayerChar)
	{
		return nullptr;
	}

	return PlayerChar->GetCurrentWeapon();
}

ACPP_PlayerCharacter* UCPP_GA_WeaponBase::GetPlayerCharacter() const
{
	// ActorInfoからキャラクターを取得
	if (!CurrentActorInfo)
	{
		return nullptr;
	}

	return Cast<ACPP_PlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
}

bool UCPP_GA_WeaponBase::IsWeaponValid() const
{
	ACPP_WeaponBase* Weapon = GetCurrentWeapon();
	return Weapon != nullptr;
}