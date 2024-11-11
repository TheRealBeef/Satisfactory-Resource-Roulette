#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ResourcePurityManager.h"
#include "ResourceCollectionManager.h"
#include "ResourceRouletteManager.generated.h"

UCLASS()
class RESOURCEROULETTE_API UResourceRouletteManager : public UObject
{
	GENERATED_BODY()

public:
	UResourceRouletteManager();
	void ScanWorldResourceNodes(UWorld* World);

private:
	bool bIsResourcesValidated;

	UPROPERTY()
	UResourceCollectionManager* ResourceCollectionManager;

	UPROPERTY()
	UResourcePurityManager* ResourcePurityManager;
};


class FResourceRouletteModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
