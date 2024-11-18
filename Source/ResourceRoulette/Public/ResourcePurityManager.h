#pragma once

#include "CoreMinimal.h"
#include "Resources/FGResourceNode.h"
#include "ResourceRouletteUtility.h"

#include "ResourcePurityManager.generated.h"

USTRUCT()
struct FResourcePurityZone
{
	GENERATED_BODY()

	UPROPERTY()	FVector2D Center;
	UPROPERTY()	float Radius;
	UPROPERTY() TEnumAsByte<EResourcePurity> DesiredPurity;
};

UCLASS()
class RESOURCEROULETTE_API UResourcePurityManager : public UObject
{
	GENERATED_BODY()

public:
	UResourcePurityManager();

	void SetRemainingPurityCounts(const TMap<FName, TMap<EResourcePurity, int32>>& NewPurityCounts);

	const TMap<FName, TMap<EResourcePurity, int32>>& GetRemainingPurityCounts() const;
	const TMap<FName, TMap<EResourcePurity, int32>>& GetFoundPurityCounts() const;
	EResourcePurity GetZonePurity(const FVector& Location) const;

	bool IsPurityAvailable(const FName ResourceClass, const EResourcePurity Purity) const;
	void DecrementAvailablePurities(const FName ResourceClass, const EResourcePurity Purity);
	void CollectWorldPurities(const UWorld* World);

private:
		void LogFoundPurities();
	void AddFoundPurity(FName ResourceClass, EResourcePurity Purity);
	static bool IsValidPurityEnum(EResourcePurity Purity);

	TMap<FName, TMap<EResourcePurity, int32>> FoundPurityCounts;
	TMap<FName, TMap<EResourcePurity, int32>> RemainingPurityCounts;

	UPROPERTY() TArray<FResourcePurityZone> PurityZones;
};
