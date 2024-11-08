#include "ResourceRouletteModule.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "ResourceNodeSpawner.h"
#include "ResourceNodeUtility.h"
#include "WorldSeedManager.h"
#include "ResourceRouletteSubsystem.h"

#define LOCTEXT_NAMESPACE "FResourceRouletteModule"


void FResourceRouletteModule::StartupModule()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FResourceRouletteModule::OnPostLoadMap);
}

void FResourceRouletteModule::ShutdownModule()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

}

void FResourceRouletteModule::OnPostLoadMap(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Error: LoadedWorld is null.", ELogLevel::Error);
        return;
    }

    AResourceRouletteSubsystem* Subsystem = AResourceRouletteSubsystem::Get(LoadedWorld);
    if (!Subsystem)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Error: Unable to find or initialize ResourceRouletteSubsystem.", ELogLevel::Error);
        return;
    }

    FResourceNodeUtilityLog::Get().LogMessage("OnPostLoadMap completed successfully.", ELogLevel::Debug);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FResourceRouletteModule, ResourceRoulette)
