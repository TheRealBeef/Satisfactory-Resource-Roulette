#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"

class AWorldSeedManager;

class FResourceRouletteModule : public IModuleInterface 
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override { return true; }

private:
	void OnPostLoadMap(UWorld* LoadedWorld);
	void SetupWorldSeedManager(UWorld* World);
	void UpdateResourceNodes (UWorld* World);

	TWeakObjectPtr<AWorldSeedManager> SeedManager = nullptr;
	FTimerHandle ScanTimerHandle;
};
