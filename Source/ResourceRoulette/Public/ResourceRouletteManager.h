#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ResourcePurityManager.h"
#include "ResourceCollectionManager.h"
#include "ResourceNodeRandomizer.h"
#include "ResourceNodeSpawner.h"
#include "ResourceRouletteManager.generated.h"

UCLASS()
class RESOURCEROULETTE_API UResourceRouletteManager : public UObject
{
	GENERATED_BODY()

public:
	UResourceRouletteManager();
	void Update(UWorld* World, AResourceRouletteSeedManager* SeedManager);
	void ScanWorldResourceNodes(const UWorld* World);
	void RandomizeWorldResourceNodes(UWorld* World);
	void SpawnWorldResourceNodes(UWorld* World);
	void UpdateWorldResourceNodes(const UWorld* World) const;
	void InitMeshesToDestroy();

private:
	bool bIsResourcesScanned;
	bool bIsResourcesRandomized;
	bool bIsResourcesSpawned;

	UPROPERTY()
	AResourceRouletteSeedManager* SeedManager;

	UPROPERTY()
	UResourceCollectionManager* ResourceCollectionManager;

	UPROPERTY()
	UResourcePurityManager* ResourcePurityManager;

	UPROPERTY()
	UResourceNodeRandomizer* ResourceNodeRandomizer;

	UPROPERTY()
	UResourceNodeSpawner* ResourceNodeSpawner;

	UPROPERTY()
	TArray<FResourceNodeData> NotProcessedResourceNodes;

	UPROPERTY()
	TSet<FName> MeshesToDestroy;
};


class FResourceRouletteModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
