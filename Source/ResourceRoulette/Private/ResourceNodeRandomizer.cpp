﻿#include "ResourceNodeRandomizer.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteSeedManager.h"
#include "ResourceRouletteSubsystem.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "ResourceRouletteProfiler.h"

UResourceNodeRandomizer::UResourceNodeRandomizer()
{
	CollectionManager = nullptr;
	PurityManager = nullptr;
	SeedManager = nullptr;
	GroupingRadius = 4000; //7000 is equivalent to 70m
}

/// Parent method to randomize World resources
/// @param World World context
/// @param InCollectionManager Collection manager instance 
/// @param InPurityManager Purity Manager instance
/// @param InSeedManager Seed manager instance
void UResourceNodeRandomizer::RandomizeWorldResources(const UWorld* World,
                                                      UResourceCollectionManager* InCollectionManager,
                                                      UResourcePurityManager* InPurityManager,
                                                      AResourceRouletteSeedManager* InSeedManager)
{
	RR_PROFILE();
	CollectionManager = InCollectionManager;
	PurityManager = InPurityManager;
	SeedManager = InSeedManager;
	if (!CollectionManager || !PurityManager || !SeedManager)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("RandomizeWorldResources can't find managers", ELogLevel::Error);
		return;
	}

	NotTouchedResourceNodes.Empty();
	NotProcessedSinglePossibleLocations.Empty();
	NotProcessedSingleResourceNodes.Empty();
	ProcessedResourceNodes.Empty();

	// Oops, not init to zero resulting in some variance when re-rolling and init.
	SingleNodeCounter = 0;

	// Get config options
	USessionSettingsManager* SessionSettings = GetWorld()->GetSubsystem<USessionSettingsManager>();
	bool bUsePurityExclusion = SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.UsePurityExclusion");
	bool bUseFullRandomization = SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.UseFullRandomization");
	int32 MaxNodesPerGroup = SessionSettings->GetIntOptionValue("ResourceRoulette.GroupOpt.MaxNumPerGroup");
	const int32 Seed = SeedManager->GetGlobalSeed();
	TArray<FResourceNodeData> NotProcessedResourceNodes = CollectionManager->GetCollectedResourceNodes();

	// These NotTouchedResourceNodes are the "vanilla" locations that we'll append later
	NotTouchedResourceNodes = FilterNodes(NotProcessedResourceNodes);
	SortNodes(NotProcessedResourceNodes);

	TArray<FVector> NotProcessedPossibleLocations;
	for (const FResourceNodeData& Node : NotProcessedResourceNodes)
	{
		if (UResourceRouletteUtility::IsValidFilteredResourceClass(Node.ResourceClass))
		{
			NotProcessedPossibleLocations.Add(Node.Location);
		}
	}

	NotProcessedPossibleLocations.Sort([](const FVector& A, const FVector& B) { return A.X < B.X; });
	PseudorandomizeLocations(NotProcessedPossibleLocations, Seed);

	ProcessedResourceNodes.Empty();

	for (int32 i = 0; i < NotTouchedResourceNodes.Num(); ++i)
	{
		PurityManager->DecrementAvailablePurities(NotTouchedResourceNodes[i].ResourceClass,
		                                          NotTouchedResourceNodes[i].Purity);
		ProcessedResourceNodes.Add(NotTouchedResourceNodes[i]);
	}

	ProcessNodes(NotProcessedResourceNodes, NotProcessedPossibleLocations, bUsePurityExclusion, bUseFullRandomization, MaxNodesPerGroup);

	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		ResourceRouletteSubsystem->SetSessionRandomizedResourceNodes(ProcessedResourceNodes);
	}
}

/// Processes nodes and randomize their location given an overly complex set of rules
/// There's probably a way to simplify this and get what I want
/// @param NotProcessedResourceNodes List of resource node structs to process
/// @param NotProcessedPossibleLocations and the list of locations
/// @param bUseFullRandomization
void UResourceNodeRandomizer::ProcessNodes(TArray<FResourceNodeData>& NotProcessedResourceNodes,
                                           TArray<FVector>& NotProcessedPossibleLocations, bool
                                           bUsePurityExclusion, bool bUseFullRandomization, int32 MaxNodesPerGroup)
{
	RR_PROFILE();
	// Get list of resources we shouldn't group
	static const TArray<FName> NonGroupableResources = UResourceRouletteUtility::GetNonGroupableResources();

	// Full Randomization - mostly ignores everything and just splatters nodes down like jackson pollock
	if (bUseFullRandomization)
	{
		TArray<FName> ValidResourceClasses = UResourceRouletteUtility::GetFilteredValidResourceClasses();
		if (ValidResourceClasses.Num() == 0 || NotProcessedPossibleLocations.Num() == 0)
		{
			FResourceRouletteUtilityLog::Get().LogMessage(
				"No valid resources or locations available for full randomization", ELogLevel::Error);
			return;
		}
		FRandomStream RandomStream(SeedManager->GetGlobalSeed());
		for (FVector Location : NotProcessedPossibleLocations)
		{
			FName RandomResourceClass = ValidResourceClasses[RandomStream.RandRange(0, ValidResourceClasses.Num() - 1)];
			FResourceNodeData* SourceNode = NotProcessedResourceNodes.FindByPredicate(
				[RandomResourceClass](const FResourceNodeData& Node)
				{
					return Node.ResourceClass == RandomResourceClass;
				});

			if (!SourceNode)
			{
				FResourceRouletteUtilityLog::Get().LogMessage(
					FString::Printf(
						TEXT("No source node available for resource class: %s"), *RandomResourceClass.ToString()),
					ELogLevel::Warning);
				continue;
			}
			FResourceNodeData NewNode = *SourceNode;
			NewNode.Location = Location;
			EResourcePurity RandomPurity = static_cast<EResourcePurity>(RandomStream.RandRange(
				0, static_cast<int32>(EResourcePurity::RP_MAX) - 1));
			NewNode.Purity = RandomPurity;
			ProcessedResourceNodes.Add(NewNode);
		}

		return;
	}

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
		GroupedLocations.Empty();
		GroupedLocationIndexes.Empty();
		VisitedIndexes.Empty();

		GroupLocations(StartingLocation, NotProcessedPossibleLocations, GroupedLocations, GroupedLocationIndexes,
		               VisitedIndexes, MaxNodesPerGroup);

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
		EResourcePurity AssignedPurity = AssignPurity(CurrentNodeToProcess.ResourceClass, GroupedLocations[0],
		                                              bUsePurityExclusion);
		if (AssignedPurity == EResourcePurity::RP_MAX)
		{
			// If no purity is available, skip this node ... something is wrong
			FResourceRouletteUtilityLog::Get().LogMessage(	
				FString::Printf(
					TEXT("No purity is available for node at location: %s"), *CurrentNodeToProcess.Location.ToString()),
				ELogLevel::Warning);
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
					AssignedPurity = AssignPurity(CurrentNodeToProcess.ResourceClass, GroupedLocations[i],
					                              bUsePurityExclusion);
					bool bPurityCheckPassed = PurityManager->IsPurityAvailable(
						CurrentNodeToProcess.ResourceClass, AssignedPurity);

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

				// Same gross TODO: stuff here as below, but ... it will work for now
				for (int32& Index : GroupedLocationIndexes)
				{
					if (Index > LocationIndex && Index < NotProcessedPossibleLocations.Num())
					{
						--Index;
					}
				}


				NotProcessedPossibleLocations.RemoveAt(LocationIndex);
				continue;
			}

			FResourceNodeData& MatchingNode = NotProcessedResourceNodes[MatchingNodeIndex];
			MatchingNode.Purity = AssignedPurity;
			MatchingNode.Location = GroupedLocations[i];
			ProcessedResourceNodes.Add(MatchingNode);
			PurityManager->DecrementAvailablePurities(MatchingNode.ResourceClass, AssignedPurity);

			int32 LocationIndex = GroupedLocationIndexes[i];

			// To resolve an issue from removing this NotProcessedPossibleLocation, the disgusting way to fix it
			// is to do this ... maybe it's better just to search and remove the location rather than this nonsense
			// but that's another TODO: item
			for (int32& Index : GroupedLocationIndexes)
			{
				if (Index > LocationIndex && Index < NotProcessedPossibleLocations.Num())
				{
					--Index;
				}
			}

			NotProcessedPossibleLocations.RemoveAt(LocationIndex);
			NotProcessedResourceNodes.RemoveAt(MatchingNodeIndex);


		}
	}

	// Add remaining locations to NotProcessedSinglePossibleLocations
	NotProcessedSinglePossibleLocations.Append(NotProcessedPossibleLocations);
	NotProcessedSingleResourceNodes.Append(NotProcessedResourceNodes);

	// Assign remaining single locations to non-groupable nodes
	for (int32 i = 0; i < NotProcessedSinglePossibleLocations.Num() && i < NotProcessedSingleResourceNodes.Num(); ++i)
	{
		NotProcessedSingleResourceNodes[i].Location = NotProcessedSinglePossibleLocations[i];
		NotProcessedSingleResourceNodes[i].Purity = AssignPurity(NotProcessedSingleResourceNodes[i].ResourceClass,
		                                                         NotProcessedSingleResourceNodes[i].Location,
		                                                         bUsePurityExclusion);
		PurityManager->DecrementAvailablePurities(NotProcessedSingleResourceNodes[i].ResourceClass,
		                                          NotProcessedSingleResourceNodes[i].Purity);
		ProcessedResourceNodes.Add(NotProcessedSingleResourceNodes[i]);
	}
}

EResourcePurity UResourceNodeRandomizer::AssignPurity(const FName ResourceClass, const FVector& NodeLocation,
                                                      bool bUsePurityExclusion) const
{
	// Check if the location falls within a purity zone
	if (bUsePurityExclusion)
	{
		EResourcePurity ZonePurity = PurityManager->GetZonePurity(NodeLocation);
		if (ZonePurity != EResourcePurity::RP_MAX && PurityManager->IsPurityAvailable(ResourceClass, ZonePurity))
		{
			return ZonePurity;
		}
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
                                             TSet<int32>& VisitedIndexes, int32 MaxNodesPerGroup)
{
	int32 StartingIndex = Locations.IndexOfByKey(StartingLocation);

	if (StartingIndex == INDEX_NONE || VisitedIndexes.Contains(StartingIndex))
	{
		return;
	}

	OutGroupedLocations.Add(StartingLocation);

	VisitedIndexes.Add(StartingIndex);
	OutGroupedIndexes.Add(StartingIndex);

	for (int32 i = 0; i < Locations.Num(); ++i)
	{
		if (OutGroupedLocations.Num() >= MaxNodesPerGroup)
		{
			break;
		}

		FVector Location = Locations[i];
		if (FVector::Dist(StartingLocation, Location) <= GroupingRadius && !VisitedIndexes.Contains(i))
		{
			GroupLocations(Location, Locations, OutGroupedLocations, OutGroupedIndexes, VisitedIndexes, MaxNodesPerGroup);
		}
	}
}

//TODO: Filters out nodes, since we no longer do it in the collection step
TArray<FResourceNodeData> UResourceNodeRandomizer::FilterNodes(TArray<FResourceNodeData>& Nodes)
{
	TArray<FResourceNodeData> NonRandomizedNodes;

	Nodes.RemoveAll([&NonRandomizedNodes](const FResourceNodeData& Node)
	{
		// Remove all non-valid classes
		if (!UResourceRouletteUtility::IsValidFilteredResourceClass(Node.ResourceClass))
		{
			NonRandomizedNodes.Add(Node);
			return true;
		}

		// Remove liquid oilfracking satellites, as they have the right class but wrong resource type
		if (Node.ResourceClass == FName("Desc_LiquidOil_C") && (Node.ResourceNodeType ==
			EResourceNodeType::FrackingSatellite || Node.ResourceNodeType == EResourceNodeType::FrackingCore))
		{
			return true;
		}
		return false;
	});
	return NonRandomizedNodes;
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
