#include "CustomResourceNode.h"
#include "ResourceNodeAssets.h"
#include "ResourceNodePurityManager.h"


FCustomResourceNode::FCustomResourceNode(const FVector& InLocation, const EOreType InOreType, const EResourcePurity InPurity)
	: Location(InLocation), OreType(InOreType), Purity(InPurity)
{
	check(FResourceNodeAssets::IsValidOreType(InOreType));
	check(UResourceNodePurityManager::IsValidPurity(InPurity));
}

void FCustomResourceNode::SetLocation(const FVector& NewLocation)
{
	Location = NewLocation;
}

void FCustomResourceNode::SetOreType(const EOreType NewOreType)
{
	if (FResourceNodeAssets::IsValidOreType(NewOreType))
	{
		OreType = NewOreType;
	}
}

void FCustomResourceNode::SetPurity(const EResourcePurity NewPurity)
{
	if (UResourceNodePurityManager::IsValidPurity(NewPurity))
	{
		Purity = NewPurity;
	}
}

FVector FCustomResourceNode::GetLocation() const
{
	return Location;
}

EOreType FCustomResourceNode::GetOreType() const
{
	return OreType;
}

EResourcePurity FCustomResourceNode::GetPurity() const
{
	return Purity;
}

bool FCustomResourceNode::operator==(const FCustomResourceNode& Other) const
{
	return Location == Other.Location && OreType == Other.OreType && Purity == Other.Purity;
}
