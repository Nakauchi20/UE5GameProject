// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CPP_GE_InitializeStats.h"
#include "GAS/CPP_PlayerAttributeSet.h"

UCPP_GE_InitializeStats::UCPP_GE_InitializeStats()
{
	// このGEは即時適用型（インスタント）
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Health 初期値のModifier
	FGameplayModifierInfo HealthMod;
	HealthMod.Attribute = UCPP_PlayerAttributeSet::GetHealthAttribute();
	HealthMod.ModifierOp = EGameplayModOp::Override;
	HealthMod.ModifierMagnitude = FScalableFloat(100.f);
	Modifiers.Add(HealthMod);

	// MaxHealth 初期値のModifier
	FGameplayModifierInfo MaxHealthMod;
	MaxHealthMod.Attribute = UCPP_PlayerAttributeSet::GetMaxHealthAttribute();
	MaxHealthMod.ModifierOp = EGameplayModOp::Override;
	MaxHealthMod.ModifierMagnitude = FScalableFloat(100.f);
	Modifiers.Add(MaxHealthMod);

	// Speed 初期値のModifier
	FGameplayModifierInfo SpeedMod;
	SpeedMod.Attribute = UCPP_PlayerAttributeSet::GetMaxSpeedAttribute();
	SpeedMod.ModifierOp = EGameplayModOp::Override;
	SpeedMod.ModifierMagnitude = FScalableFloat(600.0f);
	Modifiers.Add(SpeedMod);

}