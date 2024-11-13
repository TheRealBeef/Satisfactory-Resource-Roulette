#pragma once

#include "CoreMinimal.h"
#include "ResourceCollectionManager.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteSeedManager.h"
#include "ResourceNodeRandomizer.generated.h"

UCLASS()
class UResourceNodeRandomizer : public UObject
{
	GENERATED_BODY()

public:
	UResourceNodeRandomizer();
	void RandomizeWorldResources(UWorld* World, UResourceCollectionManager* CollectionManager,
	                             UResourcePurityManager* PurityManager, AResourceRouletteSeedManager* SeedManager);
	const TArray<FResourceNodeData>& GetProcessedNodes() const;


	float GetGroupingRadius() const;
	void SetGroupingRadius(float NewRadius);

private:
	static void FilterNodes(TArray<FResourceNodeData>& Nodes);
	static void SortNodes(TArray<FResourceNodeData>& Nodes);
	static void PseudorandomizeLocations(TArray<FVector>& Locations, int32 Seed);
	void GroupNodes(FResourceNodeData& CurrentNode, TArray<FResourceNodeData>& RemainingNodes,
	                TArray<FResourceNodeData>& GroupedNodes, TSet
	                <int32>& VisitedNodes);
	void ProcessNodes(const TArray<FResourceNodeData>& NodesToProcess, TArray<FVector>& Locations);

	UPROPERTY()
	UResourceCollectionManager* CollectionManager;

	UPROPERTY()
	UResourcePurityManager* PurityManager;

	UPROPERTY()
	AResourceRouletteSeedManager* SeedManager;

	UPROPERTY()
	TArray<FResourceNodeData> ProcessedResourceNodes;

	float GroupingRadius;
};
