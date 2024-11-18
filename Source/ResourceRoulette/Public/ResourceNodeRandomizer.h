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
	void RandomizeWorldResources(const UWorld* World, UResourceCollectionManager* InCollectionManager,
	                             UResourcePurityManager* InPurityManager, AResourceRouletteSeedManager* InSeedManager);
	const TArray<FResourceNodeData>& GetProcessedNodes() const;

	float GetGroupingRadius() const;
	void SetGroupingRadius(float NewRadius);

private:
	static void FilterNodes(TArray<FResourceNodeData>& Nodes);
	static void SortNodes(TArray<FResourceNodeData>& Nodes);
	static void PseudorandomizeLocations(TArray<FVector>& Locations, int32 Seed);
	void GroupLocations(const FVector& StartingLocation, const TArray<FVector>& Locations,
												 TArray<FVector>& OutGroupedLocations, TArray<int32>& OutGroupedIndexes,
												 TSet<int32>& VisitedIndexes);
	void ProcessNodes(TArray<FResourceNodeData>& NotProcessedResourceNodes, TArray<FVector>& NotProcessedPossibleLocations);
	EResourcePurity AssignPurity(FName ResourceClass) const;

	UPROPERTY()	UResourceCollectionManager* CollectionManager;
	UPROPERTY()	UResourcePurityManager* PurityManager;
	UPROPERTY()	AResourceRouletteSeedManager* SeedManager;
	UPROPERTY()	TArray<FResourceNodeData> ProcessedResourceNodes;
	
	TArray<FResourceNodeData> NotProcessedSingleResourceNodes;
		TArray<FVector> NotProcessedSinglePossibleLocations;

	float GroupingRadius;
	float SingleNodeSpawnChance;
	int32 SingleNodeCounter;
};
