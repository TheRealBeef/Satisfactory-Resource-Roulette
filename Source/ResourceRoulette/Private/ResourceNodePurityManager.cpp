#include "ResourceNodePurityManager.h"
#include "FGResourceNode.h"
#include "ResourceNodeUtility.h"
#include "EngineUtils.h"
#include "ResourceNodeAssets.h"

UResourceNodePurityManager::UResourceNodePurityManager()
{
}

EResourcePurity UResourceNodePurityManager::DeterminePurityToAssign(const EOreType OreType, TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts)
{
	EResourcePurity PurityToAssign = EResourcePurity::RP_Normal;

	if (RemainingPurityCounts.Contains(OreType))
	{
		auto& PurityCounts = RemainingPurityCounts[OreType];

		if (PurityCounts.Contains(EResourcePurity::RP_Pure) && PurityCounts[EResourcePurity::RP_Pure] > 0)
		{
			PurityToAssign = EResourcePurity::RP_Pure;
			PurityCounts[EResourcePurity::RP_Pure]--;
		}
		else if (PurityCounts.Contains(EResourcePurity::RP_Normal) && PurityCounts[EResourcePurity::RP_Normal] > 0)
		{
			PurityToAssign = EResourcePurity::RP_Normal;
			PurityCounts[EResourcePurity::RP_Normal]--;
		}
		else if (PurityCounts.Contains(EResourcePurity::RP_Inpure) && PurityCounts[EResourcePurity::RP_Inpure] > 0)
		{
			PurityToAssign = EResourcePurity::RP_Inpure;
			PurityCounts[EResourcePurity::RP_Inpure]--;
		}
	}
	return PurityToAssign;
}

void UResourceNodePurityManager::LogAndUpdatePurity(const EOreType OreType, const EResourcePurity Purity, TMap<EOreType, TMap<EResourcePurity, int>>& PurityCounts)
{
	PurityCounts.FindOrAdd(OreType).FindOrAdd(Purity)++;

	// Log the current purity counts
	FString OreTypeName = FResourceNodeAssets::GetOreTypeName(OreType);
	FString LogMessage = FString::Printf(
		TEXT("%s | Pure: %d | Normal: %d | Impure: %d"),
		*OreTypeName,
		PurityCounts[OreType].FindOrAdd(EResourcePurity::RP_Pure),
		PurityCounts[OreType].FindOrAdd(EResourcePurity::RP_Normal),
		PurityCounts[OreType].FindOrAdd(EResourcePurity::RP_Inpure)
	);

	FResourceNodeUtilityLog::Get().LogMessage(LogMessage,ELogLevel::Debug);
}

void UResourceNodePurityManager::CountNodePurities(const UWorld* World)
{
	OriginalPurityCounts.Empty();

	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
	{
		AFGResourceNode* ResourceNode = *It;

		if (ResourceNode->GetResourceNodeType() == EResourceNodeType::Node &&
			ResourceNode->GetResourceForm() == EResourceForm::RF_SOLID &&
			ResourceNode->GetResourceAmount() == EResourceAmount::RA_Infinite)
		{
			EOreType OreType;
			if (UResourceNodeUtility::GetOreTypeFromResourceNode(ResourceNode, OreType))
			{
				EResourcePurity Purity = ResourceNode->GetResoucePurity();
				LogAndUpdatePurity(OreType, Purity, OriginalPurityCounts);
			}
			else
			{
				FString WarningMessage = FString::Printf(TEXT("Failed to identify ore type for node: %s"), *ResourceNode->GetName());
				FResourceNodeUtilityLog::Get().LogMessage(WarningMessage, ELogLevel::Warning);
			}
		}
	}
}

void UResourceNodePurityManager::UpdatePurityCount(const EOreType OreType, const EResourcePurity Purity)
{
	IncrementPurityCount(OreType, Purity);
}

void UResourceNodePurityManager::IncrementPurityCount(const EOreType OreType, const EResourcePurity Purity)
{
	OriginalPurityCounts.FindOrAdd(OreType).FindOrAdd(Purity)++;
}

const TMap<EOreType, TMap<EResourcePurity, int>>& UResourceNodePurityManager::GetOriginalPurityCounts() const
{
	return OriginalPurityCounts;
}

bool UResourceNodePurityManager::IsValidPurity(const EResourcePurity Purity)
{
	return Purity == EResourcePurity::RP_Inpure || Purity == EResourcePurity::RP_Normal || Purity == EResourcePurity::RP_Pure;
}