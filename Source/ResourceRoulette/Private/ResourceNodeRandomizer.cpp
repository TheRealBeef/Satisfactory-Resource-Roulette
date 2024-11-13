#include "ResourceNodeRandomizer.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteSeedManager.h"

UResourceNodeRandomizer::UResourceNodeRandomizer()
{
	CollectionManager = nullptr;
	PurityManager = nullptr;
	SeedManager = nullptr;
	GroupingRadius = 7000; // Equivalent to 70m
}

void UResourceNodeRandomizer::RandomizeWorldResources(UWorld* World, UResourceCollectionManager* InCollectionManager,
                                                      UResourcePurityManager* InPurityManager,
                                                      AResourceRouletteSeedManager* InSeedManager)
{
	CollectionManager = InCollectionManager;
	PurityManager = InPurityManager;
	SeedManager = InSeedManager;
	if (!CollectionManager || !PurityManager || !SeedManager)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("RandomizeWorldResources can't find managers", ELogLevel::Error);
		return;
	}

	const int32 Seed = SeedManager->GetGlobalSeed();
	TArray<FResourceNodeData> NotProcessedResourceNodes = CollectionManager->GetCollectedResourceNodes();
	FilterNodes(NotProcessedResourceNodes);
	SortNodes(NotProcessedResourceNodes);

	TArray<FVector> PossibleLocations;
	for (const FResourceNodeData& Node : NotProcessedResourceNodes)
	{
		PossibleLocations.Add(Node.Location);
	}
	PossibleLocations.Sort([](const FVector& A, const FVector& B) { return A.X < B.X; });
	PseudorandomizeLocations(PossibleLocations, Seed);
	ProcessNodes(NotProcessedResourceNodes, PossibleLocations);

	// for (const auto& Node : ProcessedResourceNodes)
	// {
	//     FResourceRouletteUtilityLog::Get().LogMessage(
	//         FString::Printf(TEXT("Node for spawn - Class: %s, Location: %s"),
	//         *Node.ResourceClass->GetName(), *Node.Location.ToString()), ELogLevel::Debug);
	// }
}


//TODO this is temporary until I implement handling geysers and purity
void UResourceNodeRandomizer::FilterNodes(TArray<FResourceNodeData>& Nodes)
{
	Nodes.RemoveAll([](const FResourceNodeData& Node)
	{
		return Node.ResourceClass && Node.ResourceClass->GetFName() == FName("Desc_Geyser_C");
	});
}

void UResourceNodeRandomizer::SortNodes(TArray<FResourceNodeData>& Nodes)
{
	Nodes.Sort([](const FResourceNodeData& A, const FResourceNodeData& B)
	{
		return A.ResourceClass->GetFName().LexicalLess(B.ResourceClass->GetFName());
	});
}

void UResourceNodeRandomizer::PseudorandomizeLocations(TArray<FVector>& Locations, const int32 Seed)
{
	const FRandomStream RandomStream(Seed);
	for (int32 i = Locations.Num() - 1; i > 0; --i)
	{
		const int32 j = RandomStream.RandRange(0, i);
		Locations.Swap(i, j);
	}
}

void UResourceNodeRandomizer::ProcessNodes(const TArray<FResourceNodeData>& NodesToProcess, TArray<FVector>& Locations)
{
	ProcessedResourceNodes.Empty();
	TArray<FResourceNodeData> RemainingNodes = NodesToProcess;

	if (RemainingNodes.Num() == 0 || Locations.Num() == 0)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("No nodes or locations to process", ELogLevel::Warning);
		return;
	}

	TSet<int32> VisitedNodes;

	while (RemainingNodes.Num() > 0 && Locations.Num() > 0)
	{
		int32 CurrentIndex = RemainingNodes.Num() - 1;
		FResourceNodeData CurrentNode = RemainingNodes[CurrentIndex];
		RemainingNodes.Pop();

		TArray<FResourceNodeData> GroupedNodes;
		GroupNodes(CurrentNode, RemainingNodes, GroupedNodes, VisitedNodes);

		for (FResourceNodeData& Node : GroupedNodes)
		{
			if (Locations.Num() > 0)
			{
				int32 LastLocationIndex = Locations.Num() - 1;
				Node.Location = Locations[LastLocationIndex];
				Locations.RemoveAt(LastLocationIndex);
				ProcessedResourceNodes.Add(Node);
			}
			else
			{
				FResourceRouletteUtilityLog::Get().LogMessage("No locations avaialble", ELogLevel::Error);
				break;
			}
		}
	}
}

void UResourceNodeRandomizer::GroupNodes(FResourceNodeData& CurrentNode, TArray<FResourceNodeData>& RemainingNodes,
                                         TArray<FResourceNodeData>& GroupedNodes, TSet<int32>& VisitedNodes)
{
	GroupedNodes.Add(CurrentNode);

	TArray<int32> NodesToGroup;
	for (int32 i = 0; i < RemainingNodes.Num(); ++i)
	{
		if (!VisitedNodes.Contains(i) && RemainingNodes[i].ResourceClass == CurrentNode.ResourceClass &&
			FVector::Dist(RemainingNodes[i].Location, CurrentNode.Location) <= GroupingRadius)
		{
			NodesToGroup.Add(i);
			VisitedNodes.Add(i);
		}
	}

	for (int32 i : NodesToGroup)
	{
		FResourceNodeData& NeighborNode = RemainingNodes[i];
		GroupNodes(NeighborNode, RemainingNodes, GroupedNodes, VisitedNodes);
	}
}


const TArray<FResourceNodeData>& UResourceNodeRandomizer::GetProcessedNodes() const
{
	return ProcessedResourceNodes;
}

float UResourceNodeRandomizer::GetGroupingRadius() const
{
	return GroupingRadius;
}

void UResourceNodeRandomizer::SetGroupingRadius(const float NewRadius)
{
	GroupingRadius = NewRadius;
}
