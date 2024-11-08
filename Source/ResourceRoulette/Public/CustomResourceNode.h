#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"

#include "ResourceNodeUtility.h"

class FCustomResourceNode
{
public:
	FCustomResourceNode(const FVector& InLocation, EOreType InOreType, EResourcePurity InPurity);

	void SetLocation(const FVector& NewLocation);
	void SetOreType(EOreType NewOreType);
	void SetPurity(EResourcePurity NewPurity);

	FVector GetLocation() const;
	EOreType GetOreType() const;
	EResourcePurity GetPurity() const;

	bool operator==(const FCustomResourceNode& Other) const;

private:
	FVector Location;
	EOreType OreType;
	EResourcePurity Purity;
};

// Need a custom hash function here
FORCEINLINE uint32 GetTypeHash(const FCustomResourceNode& Node)
{
	return HashCombine(HashCombine(GetTypeHash(Node.GetLocation()), GetTypeHash(static_cast<uint8>(Node.GetOreType()))),
					   GetTypeHash(static_cast<uint8>(Node.GetPurity())));
}