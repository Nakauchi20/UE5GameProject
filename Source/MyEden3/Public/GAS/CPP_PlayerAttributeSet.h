// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CPP_PlayerAttributeSet.generated.h"

class ACPP_CharacterBase;

// AttributeSet.hで紹介されているアトリビュートへのSetter,Getter定義マクロ
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class MYEDEN3_API UCPP_PlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCPP_PlayerAttributeSet();

	/** レプリケーション設定 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;


	// MaxSpeed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", ReplicatedUsing = OnRep_MaxSpeed)
	FGameplayAttributeData MaxSpeed;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, MaxSpeed);

	UFUNCTION()
	void OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed);

	// MaxSpeedCrouch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", ReplicatedUsing = OnRep_MaxSpeedCrouch)
	FGameplayAttributeData MaxSpeedCrouch;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, MaxSpeedCrouch);

	UFUNCTION()
	void OnRep_MaxSpeedCrouch(const FGameplayAttributeData& OldMaxSpeedCrouch);


	// MaxHealth
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, MaxHealth)

		UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);


	// Health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, Health)

		UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	// Static attribute getters
	static FGameplayAttribute MaxSpeedAttribute();
	static FGameplayAttribute MaxSpeedCrouchAttribute();
	static FGameplayAttribute MaxHealthAttribute();
	static FGameplayAttribute HealthAttribute();

private:
	// 処理分離用ヘルパー関数
	void NotifyCharacterOfSpeedChange(float NewSpeed, bool bIsCrouchSpeed);
	void HandleHealthAttributeChange(const FGameplayEffectModCallbackData& Data, float DeltaValue);
};