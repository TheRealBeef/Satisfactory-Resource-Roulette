#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "FGBuildableResourceExtractor.h"
#include "ResourceNodeUtility.h"
#include "ResourceNodeGrouper.h"
#include "ProceduralGenerator.h"
#include "ResourceNodePurityManager.h"
#include "ResourceNodeAssets.h"
#include "ResourceNodeSpawner.generated.h"

UCLASS()
class RESOURCEROULETTE_API UResourceNodeSpawner : public UObject
{
    GENERATED_BODY()
    
public:
    UResourceNodeSpawner();

    int GetScannedUniqueResourceTypeSize() const;
    bool IsAllResourcesAlreadyScanned() const;
    static bool WorldIsLoaded(const UWorld* World);
    const TMap<EOreType, FResourceNodeAssets>& GetUniqueResourceNodeTypes();
    
    void ScanWorldResourceNodes(UWorld* World);
    void ReplaceStandardResourceNodesUpdate(UWorld* World);
    bool SpawnCustomResourceNode(UWorld* World, EOreType OreType, const FVector& Location, EResourcePurity Purity, bool bUseRaycastAdjust);
    
    void AssignMinersToNodes(UWorld* World);
    void AssignPortableMinersToNodes(UWorld* World);

    static void CollectUniqueResourceClassesAndTypes(UWorld* World, TSet<TSubclassOf<UFGResourceDescriptor>>& UniqueResourceClasses);
    static void InitializeResourceNodeComponents(AFGResourceNode* CustomNode, EOreType OreType, EResourcePurity Purity, const FVector& Location);
    void ResetResources();
    
    void ProcessResourceNodeGroups(UWorld* World, TArray<FCustomResourceNode>& StandardResourceNodes, TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts, FProceduralGenerator& Randomizer);

    void ShuffleAndReplaceNodes(UWorld* World, TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts, FProceduralGenerator& Randomizer);

    static FVector GetCenterOfVectors(const TArray<FVector>& VectorList);
    TPair<AFGResourceNode*, float> GetClosestCustomResourceNode(UWorld* World, const FVector& Location, EOccupiedType OccupiedStatus = EOccupiedType::Any);
    void SetSeed(int32 Seed);
    
private:
    
    void InitializeUniqueResourceNodeTypes(const TSet<TSubclassOf<UFGResourceDescriptor>>& UniqueResourceClasses);
    void AddResourceNodeType(EOreType OreType, TSubclassOf<UFGResourceDescriptor> ResourceClass);
    TArray<FCustomResourceNode> CollectStandardResourceNodes(UWorld* World);
    bool IsValidResourceNode(AFGResourceNode* ResourceNode) const;

    TMap<EOreType, FResourceNodeAssets> UniqueResourceNodeTypes;
    TMap<TWeakObjectPtr<AFGResourceNode>, TWeakObjectPtr<UStaticMeshComponent>> CustomResourceNodeMap;

    UClass* ResourceNodeClass = nullptr;

    UPROPERTY()
    UResourceNodePurityManager* PurityManager;
    
    FResourceNodeGrouper NodeGrouper;
    int32 SpawnerSeed;
};
