#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "ResourceNodeUtility.h"

#include "ResourceNodePurityManager.generated.h"

class UResourceNodeSpawner;

UCLASS()
class RESOURCEROULETTE_API UResourceNodePurityManager : public UObject
{
	GENERATED_BODY()

public:
	UResourceNodePurityManager();
	
	static EResourcePurity DeterminePurityToAssign(EOreType OreType, TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts);
	static void LogAndUpdatePurity(EOreType OreType, EResourcePurity Purity, TMap<EOreType, TMap<EResourcePurity, int>>& PurityCounts);

	void CountNodePurities(const UWorld* World);
	void UpdatePurityCount(EOreType OreType, EResourcePurity Purity);

	const TMap<EOreType, TMap<EResourcePurity, int>>& GetOriginalPurityCounts() const;
	static bool IsValidPurity(EResourcePurity Purity);

private:
	void IncrementPurityCount(EOreType OreType, EResourcePurity Purity);

	TMap<EOreType, TMap<EResourcePurity, int>> OriginalPurityCounts;
};
