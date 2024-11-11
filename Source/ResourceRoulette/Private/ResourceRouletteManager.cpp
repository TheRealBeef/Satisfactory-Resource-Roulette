#include "ResourceRouletteManager.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.h"


UResourceRouletteManager::UResourceRouletteManager()
{
	ResourceCollectionManager = CreateDefaultSubobject<UResourceCollectionManager>(TEXT("ResourceCollectionManager"));
	ResourcePurityManager = CreateDefaultSubobject<UResourcePurityManager>(TEXT("ResourcePurityManager"));
	bIsResourcesValidated = false;
	FResourceRouletteUtilityLog::Get().LogMessage("Resource Manager initialized successfully.", ELogLevel::Debug);
}

/// Manager method to collect resources list and purity list. Should be run 
/// only on initialization, although we may need to run it twice if some things
/// are initialized slowly
/// @param World World Context
void UResourceRouletteManager::ScanWorldResourceNodes(UWorld* World)
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("ScanWorldResourceNodes aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}
	if (!ResourceCollectionManager || !ResourcePurityManager)
	{
		return;
	}
	if (!bIsResourcesValidated)
	{

		ResourceCollectionManager->CollectWorldResources(World);
		// ResourcePurityManager->CollectWorldPurities(World);
		// bIsResourcesValidated = true;
		// FResourceRouletteUtilityLog::Get().LogMessage("Resource Scan completed successfully.", ELogLevel::Debug);
	}
	else
	{
		// FResourceRouletteUtilityLog::Get().LogMessage("New Resource scan not needed.", ELogLevel::Debug);
	}
}

#define LOCTEXT_NAMESPACE "FResourceRouletteModule"

void FResourceRouletteModule::StartupModule()
{
}

void FResourceRouletteModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FResourceRouletteModule, ResourceRoulette)
