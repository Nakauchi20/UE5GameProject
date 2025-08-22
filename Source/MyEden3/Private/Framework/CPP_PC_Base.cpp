// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CPP_PC_Base.h"
#include "EnhancedInputSubsystems.h"

ACPP_PC_Base::ACPP_PC_Base()
{

}

void ACPP_PC_Base::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(this->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}