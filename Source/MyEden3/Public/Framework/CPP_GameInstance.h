#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CPP_GameInstance.generated.h"

UCLASS()
class MYEDEN3_API UCPP_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
};