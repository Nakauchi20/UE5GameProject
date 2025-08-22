// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CPP_GA_Base.h"
#include "AbilitySystemComponent.h"

void UCPP_GA_Base::AddGameplayTags(const FGameplayTagContainer GameplayTags)
{
	UAbilitySystemComponent* Comp = GetAbilitySystemComponentFromActorInfo();

	Comp -> AddLooseGameplayTags(GameplayTags);
}

void UCPP_GA_Base::RemoveGameplayTags(const FGameplayTagContainer GameplayTags)
{
	UAbilitySystemComponent* Comp = GetAbilitySystemComponentFromActorInfo();

	Comp -> RemoveLooseGameplayTags(GameplayTags);
}
