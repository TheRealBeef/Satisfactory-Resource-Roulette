#include "ResourcePurityManager.h"
#include "Resources/FGResourceNode.h"
#include "ResourceRouletteUtility.h"
#include "EngineUtils.h"
#include "ResourceCollectionManager.h"

UResourcePurityManager::UResourcePurityManager()
{
	RemainingPurityCounts.Empty();
	FoundPurityCounts.Empty();
	
	PurityZones.Add({ FVector2D(-50000.0f, 240000.0f), 80000.0f, EResourcePurity::RP_Inpure}); // Grasslands Spawn
	PurityZones.Add({ FVector2D(50000.0f, -90000.0f), 80000.0f, EResourcePurity::RP_Inpure}); // Northern Forest Spawn
	PurityZones.Add({ FVector2D(300000.0f, -175000.0f), 120000.0f, EResourcePurity::RP_Inpure}); //Dune Desert Spawn
	PurityZones.Add({ FVector2D(-220000.0f, -35000.0f), 80000.0f, EResourcePurity::RP_Inpure}); //Rocky Desert Spawn
}

/// Gets Remaining Purity Counts
/// @return TMap of Remaining Purity Counts
const TMap<FName, TMap<EResourcePurity, int32>>& UResourcePurityManager::GetRemainingPurityCounts() const
{
	return RemainingPurityCounts;
}

/// Sets Remaining Purity Count values
/// @param NewPurityCounts TMap you'd like to overwrite it with
void UResourcePurityManager::SetRemainingPurityCounts(const TMap<FName, TMap<EResourcePurity, int32>>& NewPurityCounts)
{
	RemainingPurityCounts = NewPurityCounts;
}

/// Gets the Original "Found" Purity Count values
/// @return TMap of the found purity counts
const TMap<FName, TMap<EResourcePurity, int32>>& UResourcePurityManager::GetFoundPurityCounts() const
{
	return FoundPurityCounts;
}

EResourcePurity UResourcePurityManager::GetZonePurity(const FVector& Location) const
{
	for (const FResourcePurityZone& Zone : PurityZones)
	{
		float Distance = FVector2D::Distance(FVector2D(Location.X, Location.Y), Zone.Center);
		if (Distance <= Zone.Radius)
		{
			return Zone.DesiredPurity;
		}
	}
	return EResourcePurity::RP_MAX;
}

/// Checks to see if the requested purity is available in RemainingPurityCounts
/// @param ResourceClass FName of resource class we're checking
/// @param Purity Purity we're checking
/// @return 
bool UResourcePurityManager::IsPurityAvailable(const FName ResourceClass, const EResourcePurity Purity) const
{
	if (RemainingPurityCounts.Contains(ResourceClass))
	{
		const auto& PurityCounts = RemainingPurityCounts[ResourceClass];
		return PurityCounts.Contains(Purity) && PurityCounts[Purity] > 0;
	}
	return false;
}

/// Decrements the availability count for a given ore type and purity level in RemainingPurityCounts
/// @param ResourceClass FName of resource class assigned
/// @param Purity Purity Assigned
void UResourcePurityManager::DecrementAvailablePurities(const FName ResourceClass, const EResourcePurity Purity)
{
	if (RemainingPurityCounts.Contains(ResourceClass))
	{
		auto& PurityCounts = RemainingPurityCounts[ResourceClass];
		if (PurityCounts.Contains(Purity) && PurityCounts[Purity] > 0)
		{
			PurityCounts[Purity]--;
		}
	}
}

/// Adds the found resource to the purity count. Intended to work one at a time
/// @param ResourceClass FName of resource class assigned
/// @param Purity Purity that was found
void UResourcePurityManager::AddFoundPurity(const FName ResourceClass, const EResourcePurity Purity)
{
	if (IsValidPurityEnum(Purity))
	{
		FoundPurityCounts.FindOrAdd(ResourceClass).FindOrAdd(Purity)++;
	}
}

/// Core method of Purity Manager to iterate over all resource nodes and collect their purity levels
/// @param World - World context
void UResourcePurityManager::CollectWorldPurities(const UWorld* World)
{
	FoundPurityCounts.Empty();
	RemainingPurityCounts.Empty();

	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
	{
		AFGResourceNode* ResourceNode = *It;

		if (UResourceRouletteUtility::IsValidInfiniteResourceNode(ResourceNode))
		{
			if (ResourceNode->GetResourceClass()->GetFName() == FName("Desc_LiquidOil_C") && ResourceNode->GetResourceNodeType() == EResourceNodeType::FrackingSatellite)
			{
				continue;
			}
			
			const EResourcePurity Purity = ResourceNode->GetResoucePurity();
			const FName ResourceClassName = ResourceNode->GetResourceClass()->GetFName();
			AddFoundPurity(ResourceClassName, Purity);
		}
	}

	// LogFoundPurities();
	RemainingPurityCounts = FoundPurityCounts;
}

/// Collects purity data from array of FResourceNodeData instead of from the world
/// @param ResourceNodes Array of FResourceNodeData
void UResourcePurityManager::CollectOriginalPurities(const TArray<FResourceNodeData>& ResourceNodes)
{
	FoundPurityCounts.Empty();
	RemainingPurityCounts.Empty();

	for (const FResourceNodeData& NodeData : ResourceNodes)
	{
		if (!IsValidPurityEnum(NodeData.Purity))
		{
			continue;
		}
		const FName& ResourceClassName = NodeData.ResourceClass;
		AddFoundPurity(ResourceClassName, NodeData.Purity);
	}

	RemainingPurityCounts = FoundPurityCounts;
	
	// LogFoundPurities();
}


/// Logs the purity values
void UResourcePurityManager::LogFoundPurities()
{
	for (const auto& ResourcePair : FoundPurityCounts)
	{
		const FName& ResourceClass = ResourcePair.Key;
		const int32 PureCount = ResourcePair.Value.Find(EResourcePurity::RP_Pure)
			                        ? *ResourcePair.Value.Find(EResourcePurity::RP_Pure)
			                        : 0;
		const int32 NormalCount = ResourcePair.Value.Find(EResourcePurity::RP_Normal)
			                          ? *ResourcePair.Value.Find(EResourcePurity::RP_Normal)
			                          : 0;
		const int32 ImpureCount = ResourcePair.Value.Find(EResourcePurity::RP_Inpure)
			                          ? *ResourcePair.Value.Find(EResourcePurity::RP_Inpure)
			                          : 0;

		const FString LogMessage = FString::Printf(
			TEXT("%s | Pure: %d | Normal: %d | Impure: %d"),
			*ResourceClass.ToString(), PureCount, NormalCount, ImpureCount
		);

		FResourceRouletteUtilityLog::Get().LogMessage(LogMessage, ELogLevel::Debug);
	}
}

/// Checks if resource purity is a valid value
/// @param Purity EResourcePurity purity to check
/// @return True if it's valid, False if it's invalid
bool UResourcePurityManager::IsValidPurityEnum(const EResourcePurity Purity)
{
	return Purity == EResourcePurity::RP_Inpure || Purity == EResourcePurity::RP_Normal || Purity ==
		EResourcePurity::RP_Pure;
}
