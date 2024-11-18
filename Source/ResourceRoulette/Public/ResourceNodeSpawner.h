#pragma once

#include "CoreMinimal.h"
#include "ResourceAssets.h"
#include "ResourceCollectionManager.h"
#include "ResourceNodeRandomizer.h"
#include "ResourceRouletteSeedManager.h"
#include "Resources/FGResourceNode.h"
#include "ResourceNodeSpawner.generated.h"

UCLASS()
class RESOURCEROULETTE_API UResourceNodeSpawner : public UObject
{
	GENERATED_BODY()

public:
	UResourceNodeSpawner();

	void SpawnWorldResources(UWorld* World, UResourceNodeRandomizer* InNodeRandomizer, bool IsFromSaved);
	bool SpawnResourceNodeDecal(UWorld* World, FResourceNodeData& NodeData,
	                            const UResourceRouletteAssets* ResourceAssets);

	TMap<FGuid, AFGResourceNode*>& GetSpawnedResourceNodes() {return SpawnedResourceNodes;}
	
private:
	bool SpawnResourceNodeSolid(UWorld* World, FResourceNodeData& NodeData,
	                                         const UResourceRouletteAssets* ResourceAssets);

	UPROPERTY()	TMap<FGuid, AFGResourceNode*> SpawnedResourceNodes;
	UPROPERTY()	UResourceNodeRandomizer* NodeRandomizer;
	
	TArray<FResourceNodeData> ProcessedNodes;
};
