#pragma once

#include "CoreMinimal.h"
#include "Resources/FGResourceDescriptor.h"
#include "Resources/FGResourceNode.h"
#include "Resources/FGExtractableResourceInterface.h"
#include "ResourceRouletteInvalidNode.generated.h"

UCLASS()
class RESOURCEROULETTE_API UResourceRouletteInvalidResource : public UFGResourceDescriptor
{
	GENERATED_BODY()

public:
	UResourceRouletteInvalidResource()
	{
		mDisplayName = FText::FromString("Invalid - Rebuild extractor");
		mDescription = FText::FromString(
			"The extractor's resource has changed. You must destroy it and rebuild a new one.");
		mForm = EResourceForm::RF_SOLID;
	}
};

UCLASS()
class RESOURCEROULETTE_API AResourceRouletteInvalidNode : public AFGResourceNode
{
	GENERATED_BODY()

public:
	AResourceRouletteInvalidNode()
	{
		mCanPlaceResourceExtractor = false;
		mPurity = EResourcePurity::RP_Normal;
		mAmount = EResourceAmount::RA_Normal;
		mResourceClass = UResourceRouletteInvalidResource::StaticClass();
		mIsOccupied = false;
		mResourceNodeType = EResourceNodeType::Node;
	}

	virtual void SetIsOccupied(bool occupied) override {}
	virtual bool CanBecomeOccupied() const override { return false; }
	virtual bool HasAnyResources() const override { return false; }
	// virtual int32 ExtractResource(int32 Amount) override { return 0; }
	virtual float GetExtractionSpeedMultiplier() const override { return 0.0f; }
	virtual FVector GetPlacementLocation(const FVector& hitLocation) const override { return FVector::ZeroVector; }
	virtual bool CanPlaceResourceExtractor() const override { return false; }

	virtual TSubclassOf<UFGResourceDescriptor> GetResourceClass() const override
	{
		return UResourceRouletteInvalidResource::StaticClass();
	}

protected:
	virtual void BeginPlay() override
	{
		Super::BeginPlay();
	}
};
