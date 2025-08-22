// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h" 
#include "CPP_PlayerAttributeSet.generated.h"


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
	// 初期値設定用コンストラクタ定義
	UCPP_PlayerAttributeSet();

	/** レプリケーション設定 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** エフェクトによりアトリビュートが変化した場合のPost処理。主にUE5で直接管理しているメンバへの書き戻しを行う　*/
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;


	// MaxSpeed - レプリケーション対応
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", ReplicatedUsing = OnRep_MaxSpeed)
	FGameplayAttributeData MaxSpeed;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, MaxSpeed);
		FGameplayAttribute MaxSpeedAttribute();
    UFUNCTION()		// レプリケーション通知関数
    void OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed);


	// MaxHealth - レプリケーション対応
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, MaxHealth)
		FGameplayAttribute MaxHealthAttribute();
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);


	// Health - レプリケーション対応
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCPP_PlayerAttributeSet, Health)
		FGameplayAttribute HealthAttribute();
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
};