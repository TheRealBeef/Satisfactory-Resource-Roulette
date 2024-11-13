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

	void SpawnWorldResources(UWorld* World, UResourceNodeRandomizer* InNodeRandomizer);

private:
	static bool SpawnCustomResourceNodeSolid(UWorld* World, const FResourceNodeData& NodeData,
	                                         const UResourceRouletteAssets* ResourceAssets);

	UPROPERTY()
	TMap<AFGResourceNode*, UStaticMeshComponent*> CustomResourceNodeMap;

	UPROPERTY()
	UResourceNodeRandomizer* NodeRandomizer;
};
