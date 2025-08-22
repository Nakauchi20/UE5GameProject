// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CPP_GM_Base.h"
#include "Characters/CPP_CharacterBase.h"
#include "Framework/CPP_PlayerStateBase.h"
#include "GameFramework/PlayerController.h"

ACPP_GM_Base::ACPP_GM_Base()
{
    // デフォルトクラスの設定
    DefaultPawnClass = ACPP_CharacterBase::StaticClass();
    PlayerStateClass = ACPP_PlayerStateBase::StaticClass();

    // 必要に応じて他のクラスも設定
    // PlayerControllerClass = ACPP_PlayerController::StaticClass();
}
