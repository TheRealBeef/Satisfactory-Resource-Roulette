﻿#pragma once

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
	void Update(UWorld* World, AResourceRouletteSeedManager* InSeedManager, bool bReroll = false);
	void ScanWorldResourceNodes(UWorld* World, bool bReroll = false);
	void RandomizeWorldResourceNodes(UWorld* World, bool bReroll = false);
	void SpawnWorldResourceNodes(UWorld* World, bool IsFromSaved);
	void UpdateWorldResourceNodes(const UWorld* World) const;
	void InitMeshesToDestroy();
	void RemoveResourceRouletteNodes();
	void UpdateRadarTowers() const;
	void RemoveExtractorsFromWorld() const;

private:
	// Used in the mesh destroying bonanza
	mutable FCriticalSection CriticalSection;

	bool bIsResourcesScanned;
	bool bIsResourcesRandomized;
	bool bIsResourcesSpawned;

	UPROPERTY()	AResourceRouletteSeedManager* SeedManager;
	UPROPERTY()	UResourceCollectionManager* ResourceCollectionManager;
	UPROPERTY()	UResourcePurityManager* ResourcePurityManager;
	UPROPERTY()	UResourceNodeRandomizer* ResourceNodeRandomizer;
	UPROPERTY()	UResourceNodeSpawner* ResourceNodeSpawner;
	UPROPERTY()	TArray<FResourceNodeData> NotProcessedResourceNodes;
	UPROPERTY()	TSet<FName> RegisteredTags;
	UPROPERTY()	TSet<FName> MeshesToDestroy;
};

// TODO: I should probably fix this
class FResourceRouletteModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
