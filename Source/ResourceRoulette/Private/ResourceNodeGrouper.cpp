#include "ResourceNodeGrouper.h"

void FResourceNodeGrouper::AddResourceNode(const FCustomResourceNode& ResourceNode)
{
    EOreType OreType = ResourceNode.GetOreType();
    StandardResourceNodeList.FindOrAdd(OreType).Add(ResourceNode);

    EResourcePurity Purity = ResourceNode.GetPurity();
    TMap<EResourcePurity, int>& PurityMap = OreTypeWithPurity.FindOrAdd(OreType);
    PurityMap.FindOrAdd(Purity)++;
}

void FResourceNodeGrouper::OrganizeResourceNodeGroups(const float DistanceToGroup)
{
    for (auto& ResourceNodePair : StandardResourceNodeList)
    {
        ResourceNodePair.Value.Sort([](const FCustomResourceNode& A, const FCustomResourceNode& B)
        {
            return A.GetLocation().X < B.GetLocation().X;
        });
    }
    ClusterNearbyNodes(DistanceToGroup);
    RemoveGroupedNodesFromStandardList();
}

void FResourceNodeGrouper::ClusterNearbyNodes(const float DistanceToGroup)
{
    ResourceNodeGroups.Empty();
    TSet<FCustomResourceNode> AlreadyGroupedNodes;

    for (auto& ResourceNodePair : StandardResourceNodeList)
    {
        for (const auto& ResourceNode : ResourceNodePair.Value)
        {
            if (AlreadyGroupedNodes.Contains(ResourceNode))
                continue;

            TArray<FCustomResourceNode> NewGroup = { ResourceNode };
            AlreadyGroupedNodes.Add(ResourceNode);

            for (const auto& AnotherResourceNode : ResourceNodePair.Value)
            {
                if (ResourceNode == AnotherResourceNode || AlreadyGroupedNodes.Contains(AnotherResourceNode))
                    continue;

                if (FVector::Dist(AnotherResourceNode.GetLocation(), ResourceNode.GetLocation()) <= DistanceToGroup)
                {
                    NewGroup.Add(AnotherResourceNode);
                    AlreadyGroupedNodes.Add(AnotherResourceNode);
                }
            }

            if (NewGroup.Num() > 1)
            {
                ResourceNodeGroups.Add(NewGroup);
            }
        }
    }
}

void FResourceNodeGrouper::RemoveGroupedNodesFromStandardList()
{
    for (const auto& Group : ResourceNodeGroups)
    {
        for (const auto& Node : Group)
        {
            if (auto* NodeList = StandardResourceNodeList.Find(Node.GetOreType()))
            {
                NodeList->RemoveSingle(Node);
                if (NodeList->IsEmpty())
                {
                    StandardResourceNodeList.Remove(Node.GetOreType());
                }
            }
        }
    }
}

bool FResourceNodeGrouper::IsNodeInAnyGroup(const FCustomResourceNode& Node) const
{
    for (const auto& Group : ResourceNodeGroups)
    {
        if (Group.Contains(Node))
        {
            return true;
        }
    }
    return false;
}

const TMap<EOreType, TMap<EResourcePurity, int>>& FResourceNodeGrouper::GetOreTypeWithPurity() const
{
    return OreTypeWithPurity;
}

const TArray<TArray<FCustomResourceNode>>& FResourceNodeGrouper::GetResourceNodeGroups() const
{
    return ResourceNodeGroups;
}

const TMap<EOreType, TArray<FCustomResourceNode>>& FResourceNodeGrouper::GetStandardResourceNodeList() const
{
    return StandardResourceNodeList;
}
