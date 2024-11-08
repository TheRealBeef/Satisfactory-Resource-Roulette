#pragma once

#include "CoreMinimal.h"
#include "CustomResourceNode.h"
#include "Containers/Array.h"
#include "Containers/Map.h"

#include "ResourceNodeUtility.h"

class FResourceNodeGrouper
{
public:
    void AddResourceNode(const FCustomResourceNode& ResourceNode);
    void OrganizeResourceNodeGroups(float DistanceToGroup);

    const TMap<EOreType, TMap<EResourcePurity, int>>& GetOreTypeWithPurity() const;
    const TArray<TArray<FCustomResourceNode>>& GetResourceNodeGroups() const;
    const TMap<EOreType, TArray<FCustomResourceNode>>& GetStandardResourceNodeList() const;

private:
    void ClusterNearbyNodes(float DistanceToGroup);
    void RemoveGroupedNodesFromStandardList();
    bool IsNodeInAnyGroup(const FCustomResourceNode& Node) const;
    
    TMap<EOreType, TMap<EResourcePurity, int>> OreTypeWithPurity;
    TArray<TArray<FCustomResourceNode>> ResourceNodeGroups;
    TMap<EOreType, TArray<FCustomResourceNode>> StandardResourceNodeList;
};
