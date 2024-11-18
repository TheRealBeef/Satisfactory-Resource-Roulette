#include "ResourceNodeRandomizer.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteSeedManager.h"
#include "ResourceRouletteSubsystem.h"

UResourceNodeRandomizer::UResourceNodeRandomizer()
{
	CollectionManager = nullptr;
	PurityManager = nullptr;
	SeedManager = nullptr;
	GroupingRadius = 4000; //7000 is equivalent to 70m
	SingleNodeCounter = 0;
}

/// Parent method to randomize World resources
/// @param World World context
/// @param InCollectionManager Collection manager instance 
/// @param InPurityManager Purity Manager instance
/// @param InSeedManager Seed manager instance
void UResourceNodeRandomizer::RandomizeWorldResources(const UWorld* World, UResourceCollectionManager* InCollectionManager,
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

	TArray<FVector> NotProcessedPossibleLocations;
	for (const FResourceNodeData& Node : NotProcessedResourceNodes)
	{
		NotProcessedPossibleLocations.Add(Node.Location);
	}
	NotProcessedPossibleLocations.Sort([](const FVector& A, const FVector& B) { return A.X < B.X; });
	PseudorandomizeLocations(NotProcessedPossibleLocations, Seed);
	
	ProcessNodes(NotProcessedResourceNodes, NotProcessedPossibleLocations);
		
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		ResourceRouletteSubsystem->SetSessionRandomizedResourceNodes(ProcessedResourceNodes);
	}
	// for (const auto& Node : ProcessedResourceNodes)
	// {
	//     FResourceRouletteUtilityLog::Get().LogMessage(
	//         FString::Printf(TEXT("Node for spawn - Class: %s, Location: %s"),
	//         *Node.ResourceClass->GetName(), *Node.Location.ToString()), ELogLevel::Debug);
	// }
}

/// Processes nodes and randomize their location given an overly complex set of rules
/// There's probably a way to simplify this and get what I want
/// TODO need to incorporate the purity manager and add some "impure regions" where
/// we can't spawn pure nodes, e.g. grasslands where it becomes too easy
/// @param NotProcessedResourceNodes List of resource node structs to process
/// @param NotProcessedPossibleLocations and the list of locations
void UResourceNodeRandomizer::ProcessNodes(TArray<FResourceNodeData>& NotProcessedResourceNodes, TArray<FVector>& NotProcessedPossibleLocations)
{
    ProcessedResourceNodes.Empty();

	// TODO this should be set by configuration instead of hardcoded
	static const TArray<FName> NonGroupableResources = { "Desc_SAM_C", "Desc_OreBauxite_C", "Desc_OreUranium_C", "Desc_RP_Thorium_C" };
    
    while (NotProcessedResourceNodes.Num() > 0 && NotProcessedPossibleLocations.Num() > 0)
    {
        if (NotProcessedResourceNodes.Num() == 0 || NotProcessedPossibleLocations.Num() == 0)
        {
	        break;
        }
        FResourceNodeData CurrentNodeToProcess = NotProcessedResourceNodes.Last();

    	// If it shouldn't be grouped, then:
        if (NonGroupableResources.Contains(CurrentNodeToProcess.ResourceClass))
        {
            // Remove this node from the list to group and add it to single nodes
            NotProcessedSingleResourceNodes.Add(CurrentNodeToProcess);
            NotProcessedResourceNodes.Pop();
            continue;
        }
    	
        FVector StartingLocation = NotProcessedPossibleLocations.Last();

        // recursively find all the node locations next to this location
        TArray<FVector> GroupedLocations;
        TArray<int32> GroupedLocationIndexes;
    	TSet<int32> VisitedIndexes;
    	GroupLocations(StartingLocation, NotProcessedPossibleLocations, GroupedLocations, GroupedLocationIndexes, VisitedIndexes);

        if (GroupedLocations.Num() == 1)
        {
            SingleNodeCounter++;
        	// 25% chance, process it anyways, or 75% chance we do inside the if statement.
        	// It's not "really" random but its repeatable
            if (SingleNodeCounter % 4 != 0)
            {
                NotProcessedSinglePossibleLocations.Add(GroupedLocations[0]);
                NotProcessedPossibleLocations.Pop(); // we processed the last location so this should work
                continue;
            }
        }

    	// Assign the first location to the node and add it to ProcessedResourceNodes, also assign purity stuff
    	EResourcePurity AssignedPurity = AssignPurity(CurrentNodeToProcess.ResourceClass, GroupedLocations[0]);
    	if (AssignedPurity == EResourcePurity::RP_MAX)
    	{
    		// If no purity is available, skip this node ... something is wrong
    		FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("No purity is available for node at location: %s"), *CurrentNodeToProcess.Location.ToString()),ELogLevel::Warning);
    		continue;
    	}
    	CurrentNodeToProcess.Purity = AssignedPurity;
    	PurityManager->DecrementAvailablePurities(CurrentNodeToProcess.ResourceClass, AssignedPurity);
        CurrentNodeToProcess.Location = GroupedLocations[0];
        ProcessedResourceNodes.Add(CurrentNodeToProcess);
        NotProcessedPossibleLocations.Pop();
        NotProcessedResourceNodes.Pop();
    	
    	// Process additional locations in the group
    	for (int32 i = 1; i < GroupedLocations.Num(); ++i)
    	{
    		int32 MatchingNodeIndex = INDEX_NONE;
    		for (int32 NodeIndex = 0; NodeIndex < NotProcessedResourceNodes.Num(); ++NodeIndex)
    		{
    			const FResourceNodeData& Node = NotProcessedResourceNodes[NodeIndex];
    			if (Node.ResourceClass == CurrentNodeToProcess.ResourceClass)
    			{
    		// 		bool bPurityCheckPassed = 
						// (CurrentNodeToProcess.Purity == EResourcePurity::RP_Pure && Node.Purity != EResourcePurity::RP_Inpure) ||
						// (CurrentNodeToProcess.Purity == EResourcePurity::RP_Inpure && Node.Purity != EResourcePurity::RP_Pure) ||
						// (CurrentNodeToProcess.Purity == EResourcePurity::RP_Normal && Node.Purity == EResourcePurity::RP_Inpure) ||
						// (CurrentNodeToProcess.Purity == Node.Purity);
    				AssignedPurity = AssignPurity(CurrentNodeToProcess.ResourceClass, GroupedLocations[i]);
    				bool bPurityCheckPassed = PurityManager->IsPurityAvailable(CurrentNodeToProcess.ResourceClass, AssignedPurity);

    				if (bPurityCheckPassed)
    				{
    					MatchingNodeIndex = NodeIndex;
    					break;
    				}
    			}
    		}

    		if (MatchingNodeIndex == INDEX_NONE)
    		{
    			NotProcessedSinglePossibleLocations.Add(GroupedLocations[i]);
    			
    			int32 LocationIndex = GroupedLocationIndexes[i];
    			NotProcessedPossibleLocations.RemoveAt(LocationIndex);
    			
    			// Same gross TODO stuff here as below, but ... it will work for now
    			for (int32& Index : GroupedLocationIndexes)
    			{
    				if (Index > LocationIndex && Index < NotProcessedPossibleLocations.Num()) --Index;
    			}
    			continue;
    		}

    		FResourceNodeData& MatchingNode = NotProcessedResourceNodes[MatchingNodeIndex];
    		MatchingNode.Purity = AssignedPurity;
    		MatchingNode.Location = GroupedLocations[i];
    		ProcessedResourceNodes.Add(MatchingNode);
    		PurityManager->DecrementAvailablePurities(MatchingNode.ResourceClass, AssignedPurity);

    		int32 LocationIndex = GroupedLocationIndexes[i];
    		NotProcessedPossibleLocations.RemoveAt(LocationIndex);
    		NotProcessedResourceNodes.RemoveAt(MatchingNodeIndex);

    		// To resolve an issue from removing this NotProcessedPossibleLocation, the disgusting way to fix it
    		// is to do this ... maybe it's better just to search and remove the location rather than this nonsense
    		// but that's another TODO item
    		for (int32& Index : GroupedLocationIndexes)
    		{
    			if (Index > LocationIndex && Index < NotProcessedPossibleLocations.Num()) --Index;
    		}
    	}

    }

	// Add remaining locations to NotProcessedSinglePossibleLocations
	NotProcessedSinglePossibleLocations.Append(NotProcessedPossibleLocations);
    NotProcessedSingleResourceNodes.Append(NotProcessedResourceNodes);
	
	// Assign remaining single locations to non-groupable nodes
	for (int32 i = 0; i < NotProcessedSinglePossibleLocations.Num() && i < NotProcessedSingleResourceNodes.Num(); ++i)
	{
		NotProcessedSingleResourceNodes[i].Location = NotProcessedSinglePossibleLocations[i];
		NotProcessedSingleResourceNodes[i].Purity = AssignPurity(NotProcessedSingleResourceNodes[i].ResourceClass, NotProcessedSingleResourceNodes[i].Location);
		PurityManager->DecrementAvailablePurities(NotProcessedSingleResourceNodes[i].ResourceClass, NotProcessedSingleResourceNodes[i].Purity);
		ProcessedResourceNodes.Add(NotProcessedSingleResourceNodes[i]);
	}
}

EResourcePurity UResourceNodeRandomizer::AssignPurity(const FName ResourceClass, const FVector& NodeLocation) const
{
	// Check if the location falls within a purity zone
	EResourcePurity ZonePurity = PurityManager->GetZonePurity(NodeLocation);
	if (ZonePurity != EResourcePurity::RP_MAX && PurityManager->IsPurityAvailable(ResourceClass, ZonePurity))
	{
		return ZonePurity;
	}

	// Fallback to general purity assignment if no zone or unavailable zone purity
	if (PurityManager->IsPurityAvailable(ResourceClass, EResourcePurity::RP_Pure))
	{
		return EResourcePurity::RP_Pure;
	}
	if (PurityManager->IsPurityAvailable(ResourceClass, EResourcePurity::RP_Normal))
	{
		return EResourcePurity::RP_Normal;
	}
	if (PurityManager->IsPurityAvailable(ResourceClass, EResourcePurity::RP_Inpure))
	{
		return EResourcePurity::RP_Inpure;
	}
	return EResourcePurity::RP_MAX;
}


void UResourceNodeRandomizer::GroupLocations(const FVector& StartingLocation, const TArray<FVector>& Locations,
											 TArray<FVector>& OutGroupedLocations, TArray<int32>& OutGroupedIndexes,
											 TSet<int32>& VisitedIndexes)
{
	int32 StartingIndex = Locations.IndexOfByKey(StartingLocation);
	OutGroupedLocations.Add(StartingLocation);

	VisitedIndexes.Add(StartingIndex);
	OutGroupedIndexes.Add(StartingIndex);

	for (int32 i = 0; i < Locations.Num(); ++i)
	{
		FVector Location = Locations[i];
		if (FVector::Dist(StartingLocation, Location) <= GroupingRadius && !VisitedIndexes.Contains(i))
		{
			GroupLocations(Location, Locations, OutGroupedLocations, OutGroupedIndexes, VisitedIndexes);
		}
	}
}

//TODO this is temporary until I implement handling geysers and purity
void UResourceNodeRandomizer::FilterNodes(TArray<FResourceNodeData>& Nodes)
{
	Nodes.RemoveAll([](const FResourceNodeData& Node)
	{
		// Remove if no resource class and geysers
		if (Node.ResourceClass.IsNone() || Node.ResourceClass == FName("Desc_Geyser_C"))
		{
			return true;
		}

		// Remove liquid oilfracking satellites
		if (Node.ResourceClass == FName("Desc_LiquidOil_C") && Node.ResourceNodeType == EResourceNodeType::FrackingSatellite)
		{
			return true;
		}
		return false;
	});
}



/// Sorts primarily by ResourceClass and secondarily by Purity, in Pure/Normal/Impure order
/// @param Nodes Nodes to Sort
void UResourceNodeRandomizer::SortNodes(TArray<FResourceNodeData>& Nodes)
{
	Nodes.Sort([](const FResourceNodeData& A, const FResourceNodeData& B)
	{
		if (A.ResourceClass != B.ResourceClass)
		{
			return A.ResourceClass.LexicalLess(B.ResourceClass);
		}
		return A.Purity > B.Purity;
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
