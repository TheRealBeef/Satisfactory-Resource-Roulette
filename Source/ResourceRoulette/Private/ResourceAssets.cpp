#include "ResourceAssets.h"

const TMap<FName, FResourceRouletteAssetSolid> UResourceRouletteAssets::SolidResourceInfoMap =
{
	{
		"Desc_OreIron_C", FResourceRouletteAssetSolid{
			"/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreIron_01.ResourceNode_OreIron_01",
			{"/Game/FactoryGame/Resource/RawResources/OreIron/Material/ResourceNode_Iron_Inst.ResourceNode_Iron_Inst"},
			FVector(-400.0f, 0.0f, -40.0f),
			FVector(2.0f, 2.0f, 2.5f)
		}
	},
	{
		"Desc_OreCopper_C", FResourceRouletteAssetSolid{
			"/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreCopper_01.ResourceNode_OreCopper_01",
			{
				"/Game/FactoryGame/Resource/RawResources/OreCopper/Material/ResourceNode_Copper_Inst.ResourceNode_Copper_Inst"
			},
			FVector(-400.0f, 0.0f, -40.0f),
			FVector(2.0f, 2.0f, 2.5f)
		}
	},
	{
		"Desc_OreGold_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreGold_01.ResourceNode_OreGold_01",
			{"/Game/FactoryGame/Resource/RawResources/OreGold/Material/ResourceNode_Gold_Inst.ResourceNode_Gold_Inst"},
			FVector(-400.0f, 0.0f, -40.0f),
			FVector(2.0f, 2.0f, 2.5f))
	},
	{
		"Desc_OreBauxite_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreIron_01.ResourceNode_OreIron_01",
			{
				"/Game/FactoryGame/Resource/RawResources/OreBauxite/Material/MI_ResourceNode_OreBauxite.MI_ResourceNode_OreBauxite",
				"/Game/FactoryGame/Resource/RawResources/OreBauxite/Material/MI_ResourceNode_Middle_OreBauxite.MI_ResourceNode_Middle_OreBauxite"
			},
			FVector(-400.0f, 0.0f, -40.0f),
			FVector(2.0f, 2.0f, 2.5f))
	},
	{
		"Desc_OreUranium_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_OreIron_01.ResourceNode_OreIron_01",
			{
				"/Game/FactoryGame/Resource/RawResources/OreUranium/Material/ResourceNode_OreUranium_Inst.ResourceNode_OreUranium_Inst",
				"/Game/FactoryGame/Resource/RawResources/OreUranium/Material/MI_ResourceNode_Middle_OreUranium.MI_ResourceNode_Middle_OreUranium"
			},
			FVector(-400.0f, 0.0f, -40.0f),
			FVector(2.0f, 2.0f, 2.5f))
	},
	{
		"Desc_RawQuartz_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/ResourceNode_Quartz.ResourceNode_Quartz",
			{
				"/Game/FactoryGame/Resource/RawResources/OreQuartz/Material/ResourceNode_Quartz_Inst.ResourceNode_Quartz_Inst"
			},
			FVector(-400.0f, 0.0f, -40.0f),
			FVector(2.0f, 2.0f, 2.5f))
	},
	{
		"Desc_Sulfur_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/SulfurResource_01.SulfurResource_01",
			{"/Game/FactoryGame/Resource/RawResources/Sulfur/Material/Resource_Sulfur_Inst.Resource_Sulfur_Inst"},
			FVector(0.0f, 0.0f, -10.0f),
			FVector(1.0f, 1.0f, 1.0f))
	},
	{
		"Desc_Coal_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/CoalResource_01.CoalResource_01",
			{"/Game/FactoryGame/Resource/RawResources/Coal/Material/CoalResource_01_Inst.CoalResource_01_Inst"},
			FVector(0.0f, 0.0f, -10.0f),
			FVector(1.0f, 1.0f, 1.0f))
	},
	{
		"Desc_Stone_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/Nodes/Resource_Stone_01.Resource_Stone_01",
			{
				"/Game/FactoryGame/Resource/RawResources/Stone/Material/MI_ResourceNode_Stone_Blocks.MI_ResourceNode_Stone_Blocks"
			},
			FVector(0.0f, 0.0f, -5.0f),
			FVector(2.4f, 2.4f, 2.0f))
	},
	{
		"Desc_SAM_C", FResourceRouletteAssetSolid(
			"/Game/FactoryGame/Resource/RawResources/SAM/Mesh/SM_SAM_Node_01.SM_SAM_Node_01",
			{"/Game/FactoryGame/Resource/RawResources/SAM/Material/MI_SAM_Node_01.MI_SAM_Node_01"},
			FVector(0.0f, 0.0f, 50.0f),
			FVector(1.3333f, 1.3333f, 1.8f))
	},
	{
		"Desc_RP_Thorium_C", FResourceRouletteAssetSolid{
			"/RefinedPower/World/ResourceNodes/Thorium/Mesh/SM_ThoriumNode.SM_ThoriumNode",
			{
				"/RefinedPower/World/ResourceNodes/Thorium/Materials/Mat_OreElement65_Base.Mat_OreElement65_Base",
				"/RefinedPower/World/ResourceNodes/Thorium/Materials/M_Thorium_Middle.M_Thorium_Middle"
			},
			FVector(0.0f, 0.0f, 0.0f),
			FVector(2.0f, 2.0f, 2.5f)
		}
	},
	{
		"Desc_FF_Dirt_Fertilized_C", FResourceRouletteAssetSolid{
			"/FicsitFarming/World/ResourceNodes/Dirt/Mesh/SM_DirtNode.SM_DirtNode",
			{"/FicsitFarming/World/ResourceNodes/Dirt/Material/MI_Dirt_Wet.MI_Dirt_Wet"},
			FVector(0.0f, 0.0f, 45.0f),
			FVector(1.0f, 1.0f, 0.5f)
		}
	},
	{
		"Desc_FF_Dirt_C", FResourceRouletteAssetSolid{
			"/FicsitFarming/World/ResourceNodes/Dirt/Mesh/SM_DirtNode.SM_DirtNode",
			{"/FicsitFarming/World/ResourceNodes/Dirt/Material/MI_Dirt_Normal.MI_Dirt_Normal"},
			FVector(0.0f, 0.0f, 45.0f),
			FVector(1.0f, 1.0f, 0.5f)
		}
	},
	{
		"Desc_FF_Dirt_Wet_C", FResourceRouletteAssetSolid{
			"/FicsitFarming/World/ResourceNodes/Dirt/Mesh/SM_DirtNode.SM_DirtNode",
			{"/FicsitFarming/World/ResourceNodes/Dirt/Material/MI_Dirt_Fert.MI_Dirt_Fert"},
			FVector(0.0f, 0.0f, 45.0f),
			FVector(1.0f, 1.0f, 0.5f)
		}
	}

};

const TMap<FName, FResourceRouletteAssetHeat> UResourceRouletteAssets::HeatResourceInfoMap = {
	{
		"Desc_Geyser_C", FResourceRouletteAssetHeat{
			"/Game/FactoryGame/World/Environment/HotSpring/Mesh/Hotspring_Blob_01.Hotspring_Blob_01",
			{"/Game/FactoryGame/World/Environment/HotSpring/Material/MI_Geyser.MI_Geyser"},
			FVector(0.0f, 0.0f, 100.0f),
			FVector(3.0f, 3.0f, 3.0f)
		}
	}
};



const TMap<FName, FResourceRouletteAssetLiquid> UResourceRouletteAssets::LiquidResourceInfoMap = {
	{
		"Desc_LiquidOil_C", FResourceRouletteAssetLiquid{
			{"/Game/FactoryGame/Resource/RawResources/CrudeOil/Material/CrudeOil_Puddle.CrudeOil_Puddle"},
			500.0f
		}
	}
};



const TMap<FName, FResourceRouletteAssetFracking> UResourceRouletteAssets::FrackingResourceInfoMap = {
	{
		"Desc_NitrogenGas_C", FResourceRouletteAssetFracking{
			{
				"/Game/Developers/gabrielestigliano/NitrogenNode/SM_Nitrogen_Node_Small.SM_Nitrogen_Node_Small",
				"/Game/Developers/gabrielestigliano/NitrogenNode/SM_Nitrogen_Node_Medium.SM_Nitrogen_Node_Medium"
			},
			{
				"/Game/FactoryGame/Resource/RawResources/FrackingNode/Material/MM_FrackingHole_01.MM_FrackingHole_01",
				"/Game/Developers/gabrielestigliano/NitrogenNode/MM_NitrogenJets.MM_NitrogenJets",
				"/Game/FactoryGame/Resource/RawResources/Nitrogen/Material/MM_NitrogenBubble.MM_NitrogenBubble"
			},
			FVector(0.0f, 0.0f, 0.0f),
			FVector(1.0f, 1.0f, 1.0f)
		}
	},
	{
		"Desc_RP_Deanium_C", FResourceRouletteAssetFracking{
			{"/Game/FactoryGame/Resource/RawResources/FrackingNode/SM_FrackingNode_Small_01.SM_FrackingNode_Small_01"},
			{
				"/Game/FactoryGame/Resource/RawResources/FrackingNode/Material/MI_FrackingNodes_Water_01.MI_FrackingNodes_Water_01",
				"/Game/FactoryGame/Resource/RawResources/FrackingNode/Material/MI_FrackingHole_01.MI_FrackingHole_01"
			},
			FVector(0.0, 0.0, 0.0),
			FVector(1.0f, 1.0f, 1.0f)
		}
	},
};

// Solid resource getters
FString UResourceRouletteAssets::GetSolidMesh(const FName& ResourceClass)
{
	if (SolidResourceInfoMap.Contains(ResourceClass))
	{
		return SolidResourceInfoMap[ResourceClass].MeshPath;
	}
	return FString();
}

TArray<FString> UResourceRouletteAssets::GetSolidMaterial(const FName& ResourceClass)
{
	if (SolidResourceInfoMap.Contains(ResourceClass))
	{
		return SolidResourceInfoMap[ResourceClass].MaterialPaths;
	}
	return TArray<FString>();
}

FVector UResourceRouletteAssets::GetSolidOffset(const FName& ResourceClass)
{
	if (SolidResourceInfoMap.Contains(ResourceClass))
	{
		return SolidResourceInfoMap[ResourceClass].MeshOffset;
	}
	return FVector::ZeroVector;
}

FVector UResourceRouletteAssets::GetSolidScale(const FName& ResourceClass)
{
	if (SolidResourceInfoMap.Contains(ResourceClass))
	{
		return SolidResourceInfoMap[ResourceClass].MeshScale;
	}
	return FVector(1.0f, 1.0f, 1.0f);
}

// Heat resource getters
FString UResourceRouletteAssets::GetHeatMesh(const FName& ResourceClass)
{
	if (HeatResourceInfoMap.Contains(ResourceClass))
	{
		return HeatResourceInfoMap[ResourceClass].MeshPath;
	}
	return FString();
}

TArray<FString> UResourceRouletteAssets::GetHeatMaterials(const FName& ResourceClass)
{
	if (HeatResourceInfoMap.Contains(ResourceClass))
	{
		return HeatResourceInfoMap[ResourceClass].MaterialPaths;
	}
	return TArray<FString>();
}

FVector UResourceRouletteAssets::GetHeatOffset(const FName& ResourceClass)
{
	if (HeatResourceInfoMap.Contains(ResourceClass))
	{
		return HeatResourceInfoMap[ResourceClass].MeshOffset;
	}
	return FVector::ZeroVector;
}

FVector UResourceRouletteAssets::GetHeatScale(const FName& ResourceClass)
{
	if (HeatResourceInfoMap.Contains(ResourceClass))
	{
		return HeatResourceInfoMap[ResourceClass].MeshScale;
	}
	return FVector(1.0f, 1.0f, 1.0f);
}

// Liquid resource getters
TArray<FString> UResourceRouletteAssets::GetLiquidMaterials(const FName& ResourceClass)
{
	if (LiquidResourceInfoMap.Contains(ResourceClass))
	{
		return LiquidResourceInfoMap[ResourceClass].MaterialPaths;
	}
	return TArray<FString>();
}

float UResourceRouletteAssets::GetLiquidDecalScale(const FName& ResourceClass)
{
	if (LiquidResourceInfoMap.Contains(ResourceClass))
	{
		return LiquidResourceInfoMap[ResourceClass].DecalSize;
	}
	return 0.0f;
}

// Fracking resource getters
TArray<FString> UResourceRouletteAssets::GetFrackingMeshes(const FName& ResourceClass)
{
	if (FrackingResourceInfoMap.Contains(ResourceClass))
	{
		return FrackingResourceInfoMap[ResourceClass].MeshPaths;
	}
	return TArray<FString>();
}

TArray<FString> UResourceRouletteAssets::GetFrackingMaterials(const FName& ResourceClass)
{
	if (FrackingResourceInfoMap.Contains(ResourceClass))
	{
		return FrackingResourceInfoMap[ResourceClass].MaterialPaths;
	}
	return TArray<FString>();
}

FVector UResourceRouletteAssets::GetFrackingOffset(const FName& ResourceClass)
{
	if (FrackingResourceInfoMap.Contains(ResourceClass))
	{
		return FrackingResourceInfoMap[ResourceClass].MeshOffset;
	}
	return FVector::ZeroVector;
}

FVector UResourceRouletteAssets::GetFrackingScale(const FName& ResourceClass)
{
	if (FrackingResourceInfoMap.Contains(ResourceClass))
	{
		return FrackingResourceInfoMap[ResourceClass].MeshScale;
	}
	return FVector(1.0f, 1.0f, 1.0f);
}
