#include "ResourceNodeAssets.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

//////////////////////////////////////////////////////////
// An attempt at lazy loading ... Don't judge me
//////////////////////////////////////////////////////////
FResourceNodeAssets::FResourceNodeAssets(const EOreType InOreType, const FString& InMeshPath, const FString& InMaterialPath, const FVector& InOffset, const FVector& InScale)
	: OreType(InOreType), MeshPath(InMeshPath), MaterialPath(InMaterialPath), Offset(InOffset), Scale(InScale)
{
}

void FResourceNodeAssets::LoadAssets() const
{
	if (!Loaded)
	{
		Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
		Material = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
		Loaded = (Mesh != nullptr && Material != nullptr);

		if (!Loaded)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load assets for OreType %d"), static_cast<uint8>(OreType));
		}
	}
}

UStaticMesh* FResourceNodeAssets::GetMesh() const
{
	LoadAssets();
	return Mesh;
}

UMaterialInterface* FResourceNodeAssets::GetMaterial() const
{
	LoadAssets();
	return Material;
}

const TArray<FResourceNodeAssets>& FResourceNodeAssets::GetValidResourceNodeAssets()
{
	return ValidResourceNodeAssets;
}

FString FResourceNodeAssets::GetOreTypeName(const EOreType OreType)
{
	for (const auto& Pair : OreTypeNameList)
	{
		if (Pair.Value == OreType)
		{
			return Pair.Key;
		}
	}
	return TEXT("Unknown");
}

bool FResourceNodeAssets::IsValidOreMeshPath(const FString& MeshPath)
{
	for (auto& asset : ValidResourceNodeAssets)
	{
		if (MeshPath.Equals(asset.GetMeshPath(), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}
	return false;
}

bool FResourceNodeAssets::TryGetOreTypeByMeshPath(const FString& MeshPath, EOreType& OreType)
{
	for (auto& asset : ValidResourceNodeAssets)
	{
		if (MeshPath.Equals(asset.GetMeshPath(), ESearchCase::IgnoreCase))
		{
			OreType = asset.GetOreType();
			return true;
		}
	}
	return false;
}

bool FResourceNodeAssets::IsValidOreType(const EOreType OreType)
{
	for (const FResourceNodeAssets& Asset : ValidResourceNodeAssets)
	{
		if (Asset.GetOreType() == OreType)
		{
			return true;
		}
	}
	return false;
}

FResourceNodeConfig FResourceNodeAssets::GetResourceNodeConfigForOreType(const EOreType OreType)
{
	const TArray<FResourceNodeAssets>& ResourceNodeAssets = GetValidResourceNodeAssets();
	for (const FResourceNodeAssets& Asset : ResourceNodeAssets)
	{
		if (Asset.GetOreType() == OreType)
		{
			return FResourceNodeConfig{
				Asset.GetMeshPath(),
				Asset.GetMaterialPath(),
				Asset.GetOffset(),
				Asset.GetScale()
			};
		}
	}
	return FResourceNodeConfig();
}

/////////////////////////////////////////////////////////////////////////
// Here lies the list of ValidResourceNodeAssets ... to be updated ...
// TODO - Can we autogenerate this at runtime instead?
/////////////////////////////////////////////////////////////////////////

TArray<FResourceNodeAssets> FResourceNodeAssets::ValidResourceNodeAssets = {
		FResourceNodeAssets(EOreType::Copper,
						   "/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreCopper_01.ResourceNode_OreCopper_01",
						   "/Game/FactoryGame/Resource/RawResources/OreCopper/Material/ResourceNode_Copper_Inst.ResourceNode_Copper_Inst",
						   FVector(0.0f, 0.0f, -30.0f), FVector(2.0f, 2.0f, 2.5f)),
		FResourceNodeAssets(EOreType::Coal,
						   "/Game/FactoryGame/Resource/RawResources/Nodes/CoalResource_01.CoalResource_01",
						   "/Game/FactoryGame/Resource/RawResources/Coal/Material/CoalResource_01_Inst.Resource_Coal_Inst",
						   FVector(0.0f, 0.0f, 70.0f), FVector(1.0f, 1.0f, 1.0f)),
		FResourceNodeAssets(EOreType::Bauxite, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreIron_01.ResourceNode_OreIron_01", 
						   "/Game/FactoryGame/Resource/RawResources/OreBauxite/Material/MI_ResourceNode_OreBauxite.MI_ResourceNode_OreBauxite", 
						   FVector(0.0f, 0.0f, -30.0f), FVector(2.0f, 2.0f, 2.5f)),
		FResourceNodeAssets(EOreType::Iron, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreIron_01.ResourceNode_OreIron_01", 
						   "/Game/FactoryGame/Resource/RawResources/OreIron/Material/ResourceNode_Iron_Inst.ResourceNode_Iron_Inst", 
						   FVector(0.0f, 0.0f, -30.0f), FVector(2.0f, 2.0f, 2.5f)),
		FResourceNodeAssets(EOreType::Uranium, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreIron_01.ResourceNode_OreIron_01", 
						   "/Game/FactoryGame/Resource/RawResources/OreUranium/Material/ResourceNode_OreUranium_Inst.ResourceNode_OreUranium_Inst", 
						   FVector(0.0f, 0.0f, -30.0f), FVector(2.0f, 2.0f, 2.5f)),
		FResourceNodeAssets(EOreType::Caterium, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreGold_01.ResourceNode_OreGold_01", 
						   "/Game/FactoryGame/Resource/RawResources/OreGold/Material/ResourceNode_Gold_Inst.ResourceNode_Gold_Inst", 
						   FVector(0.0f, 0.0f, -30.0f), FVector(2.0f, 2.0f, 2.5f)),
		FResourceNodeAssets(EOreType::Quartz, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_quartz.ResourceNode_quartz", 
						   "/Game/FactoryGame/Resource/RawResources/OreQuartz/Material/ResourceNode_Quartz_Inst.ResourceNode_Quartz_Inst", 
						   FVector(0.0f, 0.0f, -30.0f), FVector(2.0f, 2.0f, 2.5f)),
		FResourceNodeAssets(EOreType::Sam, 
						   "/Game/FactoryGame/Resource/RawResources/Sam/Mesh/SM_SAM_Node_01.SM_SAM_Node_01", 
						   "/Game/FactoryGame/Resource/RawResources/Sam/Material/MI_SAM_Node_01.MI_SAM_Node_01", 
						   FVector(0.0f, 0.0f, -30.0f), FVector(1.25f, 1.25f, 1.25f)),
		FResourceNodeAssets(EOreType::Sulfur, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/SulfurResource_01.SulfurResource_01", 
						   "/Game/FactoryGame/Resource/RawResources/Sulfur/Material/Resource_Sulfur_Inst.Resource_Sulfur_Inst", 
						   FVector(0.0f, 0.0f, 70.0f), FVector(1.0f, 1.0f, 1.0f)),
		FResourceNodeAssets(EOreType::Limestone, 
						   "/Game/FactoryGame/Resource/RawResources/Nodes/Resource_Stone_01.Resource_Stone_01", 
						   "/Game/FactoryGame/Resource/RawResources/Stone/Material/MI_ResourceNode_Stone_Blocks.MI_ResourceNode_Stone_Blocks", 
						   FVector(0.0f, 0.0f, 55.0f), FVector(1.85f, 1.85f, 1.85f))
	};