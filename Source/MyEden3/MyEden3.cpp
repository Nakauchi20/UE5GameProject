#include "MyEden3.h"
#include "Modules/ModuleManager.h"

void FMyEden3Module::StartupModule()
{
    FDefaultGameModuleImpl::StartupModule();

    UE_LOG(LogTemp, Log, TEXT("[MyEden3Module] StartupModule called"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FMyEden3Module, MyEden3, "MyEden3");