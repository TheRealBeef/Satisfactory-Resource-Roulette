#include "ResourceCollectionManager.h"
#include "Resources/FGResourceNode.h"
#include "ResourceRouletteUtility.h"
#include "EngineUtils.h"
#include "FGActorRepresentationManager.h"
#include "ResourceRouletteSubsystem.h"
#include "Representation/FGResourceNodeRepresentation.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Resources/FGResourceDescriptor.h"
#include "Components/DecalComponent.h"
#include "ResourceRouletteCompatibilityManager.h"
#include "Kismet/GameplayStatics.h"
#include "ResourceRouletteProfiler.h"

UResourceCollectionManager::UResourceCollectionManager()
{
	CollectedResourceNodes.Empty();
}


/// /// Sets the array of collected resource nodes externally
/// @param InCollectedResourceNodes - to save
void UResourceCollectionManager::SetCollectedResourcesNodes(const TArray<FResourceNodeData>& InCollectedResourceNodes)
{
	CollectedResourceNodes = InCollectedResourceNodes;
}

/// Collects all the resources in the world and their basic information
/// @param World World Context
void UResourceCollectionManager::CollectWorldResources(const UWorld* World)
{
	RR_PROFILE();

	CollectedResourceNodes.Empty();
	TSet<FName> RegisteredTags = ResourceRouletteCompatibilityManager::GetRegisteredTags();

	// FResourceRouletteUtilityLog::Get().LogMessage(TEXT("CollectWorldResources called"), ELogLevel::Debug);

	// Need to kill zombie nodes :\ I don't know how they get caused just yet
	TArray<FName> ZombieNodeClassnames = {
		"Desc_FF_Dirt_Fertilized_C",
		"Desc_FF_Dirt_C",
		"Desc_FF_Dirt_Wet_C",
		"Desc_RP_Thorium_C"
	};

	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
	{
		AFGResourceNode* ResourceNode = *It;

		if (!UResourceRouletteUtility::IsValidAllInfiniteResourceNode(ResourceNode))
		{
			continue;
		}

		if (!ResourceNode->HasAnyFlags(RF_WasLoaded))
		{
			// Compare with our zombie node list
			FName NodeClassName = ResourceNode->GetResourceClass()->GetFName();
			if (ZombieNodeClassnames.Contains(NodeClassName))
			{
				// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Killing zombie node: %s at location: %s"),
				// 	*NodeClassName.ToString(),*ResourceNode->GetActorLocation().ToString()),ELogLevel::Warning);

				TArray<UStaticMeshComponent*> MeshComponents;
				ResourceNode->GetComponents(MeshComponents);
				for (UStaticMeshComponent* MeshComponent : MeshComponents)
				{
					if (MeshComponent)
					{
						MeshComponent->SetVisibility(false);
						MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						MeshComponent->DestroyComponent();
					}
				}
				TArray<UDecalComponent*> DecalComponents;
				for (UDecalComponent* DecalComponent : DecalComponents)
				{
					if (DecalComponent)
					{
						DecalComponent->SetVisibility(false);
						DecalComponent->DestroyComponent();
					}
				}
				// and then the actor
				if (AFGActorRepresentationManager* RepManager = AFGActorRepresentationManager::Get(GetWorld()))
				{
					RepManager->RemoveRepresentationOfActor(ResourceNode);
				}
				ResourceNode->Destroy();
				continue;
			}
		}

		bool bIsTagged = false;
		for (const FName& RegisteredTag : RegisteredTags)
		{
			if (ResourceNode->Tags.Contains(RegisteredTag) || ResourceNode->Tags.Contains(ResourceRouletteTag))
			{
				bIsTagged = true;
				break;
			}
		}
		if (bIsTagged)
		{
			continue;
		}

		if (ResourceNode->GetResourceClass()->GetFName() == FName("Desc_LiquidOil_C") && (ResourceNode->
			GetResourceNodeType() == EResourceNodeType::FrackingSatellite || ResourceNode->GetResourceNodeType() ==
			EResourceNodeType::FrackingCore))
		{
			continue;
		}

		// Collect node infos
		FResourceNodeData NodeData;
		NodeData.Classname = ResourceNode->GetClass()->GetName();
		NodeData.Location = ResourceNode->GetActorLocation();
		NodeData.Rotation = ResourceNode->GetActorRotation();
		NodeData.Scale = ResourceNode->GetActorScale3D();
		NodeData.Purity = ResourceNode->GetResoucePurity();
		NodeData.Amount = ResourceNode->GetResourceAmount();
		NodeData.ResourceClass = ResourceNode->GetResourceClass()->GetFName();
		NodeData.ResourceNodeType = ResourceNode->GetResourceNodeType();
		NodeData.ResourceForm = ResourceNode->GetResourceForm();
		NodeData.bCanPlaceResourceExtractor = ResourceNode->CanPlaceResourceExtractor();
		NodeData.bIsOccupied = false;
		NodeData.IsRayCasted = false;
		NodeData.NodeGUID = FGuid::NewGuid();

		// Add node data to collection
		CollectedResourceNodes.Add(NodeData);

		// Destroy the mesh and/or decal
		TArray<UStaticMeshComponent*> MeshComponents;
		ResourceNode->GetComponents(MeshComponents);
		for (UStaticMeshComponent* MeshComponent : MeshComponents)
		{
			if (MeshComponent)
			{
				MeshComponent->SetVisibility(false);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->DestroyComponent();
			}
		}
		TArray<UDecalComponent*> DecalComponents;
		for (UDecalComponent* DecalComponent : DecalComponents)
		{
			if (DecalComponent)
			{
				DecalComponent->SetVisibility(false);
				DecalComponent->DestroyComponent();
			}
		}
		// and then the actor
		ResourceNode->Destroy();
	}

	// Log and clear collected resources
	// LogCollectedResources();
}

/// Logs all the collected resources
/// This method is for the more general method, vs the one that incldues mesh data
void UResourceCollectionManager::LogCollectedResources() const
{
	for (const FResourceNodeData& NodeData : CollectedResourceNodes)
	{
		// Get the resource class name or default to "UnknownClass" if null
		FString ResourceClassName = NodeData.ResourceClass != NAME_None
			                            ? NodeData.ResourceClass.ToString()
			                            : "UnknownClass";

		// Format the general information string
		FString GeneralInfo = FString::Printf(
			TEXT(
				"Classname: %s | ResourceClass: %s | ResourceForm: %s | Location: %s | Rotation: %s | Scale: %s | "
				"Purity: %d | Amount: %d | ResourceNodeType: %s | CanPlaceExtractor: %s | IsOccupied: %s | "
				"RayCasted: %s | NodeGUID: %s"
			),
			*NodeData.Classname,
			*ResourceClassName,
			*StaticEnum<EResourceForm>()->GetNameStringByValue(static_cast<int64>(NodeData.ResourceForm)),
			*NodeData.Location.ToString(),
			*NodeData.Rotation.ToString(),
			*NodeData.Scale.ToString(),
			static_cast<int32>(NodeData.Purity),
			static_cast<int32>(NodeData.Amount),
			*StaticEnum<EResourceNodeType>()->GetNameStringByValue(static_cast<int64>(NodeData.ResourceNodeType)),
			NodeData.bCanPlaceResourceExtractor ? TEXT("Yes") : TEXT("No"),
			NodeData.bIsOccupied ? TEXT("Yes") : TEXT("No"),
			NodeData.IsRayCasted ? TEXT("Yes") : TEXT("No"),
			*NodeData.NodeGUID.ToString()
		);

		// Log the general information for this node
		FResourceRouletteUtilityLog::Get().LogMessage(GeneralInfo, ELogLevel::Debug);
	}
}


//////////////////////////////////////////////////////////////////////////////////////
/// These methods are primarily for testing / debugging to grab more detailed info
/// Not needed for actual shipping of the mod
/// //////////////////////////////////////////////////////////////////////////////////

// /// Collects mesh data resource node
// /// @param ResourceNode resource node we're checking
// /// @return FResourceNodeVisualData struct containing mesh data
// FResourceNodeVisualData UResourceCollectionManager::CollectMeshData(AFGResourceNode* ResourceNode)
// {
// 	FResourceNodeVisualData VisualData;
// 	VisualData.bIsUsingDecal = false;
//
// 	TArray<UStaticMeshComponent*> MeshComponents;
// 	ResourceNode->GetComponents(MeshComponents);
//
// 	for (UStaticMeshComponent* MeshComponent : MeshComponents)
// 	{
// 		if (UStaticMesh* ResourceMesh = MeshComponent->GetStaticMesh())
// 		{
// 			VisualData.MeshData.Mesh = ResourceMesh;
// 			VisualData.MeshData.MeshPath = ResourceMesh->GetPathName();
// 			VisualData.MeshData.Location = MeshComponent->GetComponentLocation();
// 			VisualData.MeshData.Rotation = MeshComponent->GetComponentRotation();
// 			VisualData.MeshData.Scale = MeshComponent->GetComponentScale();
//
// 			int32 NumMaterials = MeshComponent->GetNumMaterials();
// 			for (int32 i = 0; i < NumMaterials; i++)
// 			{
// 				if (UMaterialInterface* Material = MeshComponent->GetMaterial(i))
// 				{
// 					VisualData.MeshData.Materials.Add(Material);
// 					VisualData.MeshData.MaterialPaths.Add(Material->GetPathName());
// 				}
// 			}
// 		}
// 	}
//
// 	if (VisualData.MeshData.Mesh == nullptr)
// 	{
// 		if (UDecalComponent* DecalComponent = ResourceNode->FindComponentByClass<UDecalComponent>())
// 		{
// 			VisualData.bIsUsingDecal = true;
// 			VisualData.DecalData.DecalMaterial = UFGResourceDescriptor::GetDecalMaterial(ResourceNode->GetResourceClass());
// 			VisualData.DecalData.DecalSize = UFGResourceDescriptor::GetDecalSize(ResourceNode->GetResourceClass());
// 			VisualData.DecalData.Rotation = DecalComponent->GetComponentRotation();
// 			VisualData.DecalData.Scale = FVector(VisualData.DecalData.DecalSize);
// 			VisualData.DecalData.Location = DecalComponent->GetComponentLocation();
// 			VisualData.DecalData.DecalMaterialPath = VisualData.DecalData.DecalMaterial
// 				? VisualData.DecalData.DecalMaterial->GetPathName()
// 				: TEXT("No Decal");
// 		}
// 	}
// 	return VisualData;
// }


// /// Logs all the collected resources to file so we can do some data analysis
// /// Not used for gameplay so much as collecting information about resources
// void UResourceCollectionManager::LogCollectedResourcesMegaLog() const
// {
//     for (const FResourceNodeData& NodeData : CollectedResourceNodes)
//     {
//         FString ResourceClassName = NodeData.ResourceClass ? NodeData.ResourceClass->GetName() : TEXT("UnknownClass");
//         FString GeneralInfo = FString::Printf(
//             TEXT("%s | ResourceForm: %s | Location: %s | Rotation: %s | Scale: %s | Purity: %d | Amount: %d | CanPlaceExtractor: %s | IsOccupied: %s"),
//             *ResourceClassName,
//             *StaticEnum<EResourceForm>()->GetNameStringByValue(static_cast<int64>(NodeData.ResourceForm)),
//             *NodeData.Location.ToString(),
//             *NodeData.Rotation.ToString(),
//             *NodeData.Scale.ToString(),
//             static_cast<int32>(NodeData.Purity),
//             static_cast<int32>(NodeData.Amount),
//             NodeData.bCanPlaceResourceExtractor ? TEXT("Yes") : TEXT("No"),
//             NodeData.bIsOccupied ? TEXT("Yes") : TEXT("No")
//         );
//         FString VisualDataLog;
//         if (!NodeData.VisualData.bIsUsingDecal)
//         {
//             FString MaterialList = NodeData.VisualData.MeshData.MaterialPaths.Num() > 0
//                 ? FString::Join(NodeData.VisualData.MeshData.MaterialPaths, TEXT(", "))
//                 : TEXT("No Materials");
//
//             VisualDataLog = FString::Printf(
//                 TEXT("Mesh Data: Mesh Path: %s | Location: %s | Rotation: %s | Scale: %s | Materials: %s"),
//                 *NodeData.VisualData.MeshData.MeshPath,
//                 *NodeData.VisualData.MeshData.Location.ToString(),
//                 *NodeData.VisualData.MeshData.Rotation.ToString(),
//                 *NodeData.VisualData.MeshData.Scale.ToString(),
//                 *MaterialList
//             );
//         }
//         else
//         {
//             VisualDataLog = FString::Printf(
//                 TEXT("Decal Data: Decal Material Path: %s | Location: %s | Rotation: %s | Scale: %s | Decal Size: %.2f"),
//                 *NodeData.VisualData.DecalData.DecalMaterialPath,
//                 *NodeData.VisualData.DecalData.Location.ToString(),
//                 *NodeData.VisualData.DecalData.Rotation.ToString(),
//                 *NodeData.VisualData.DecalData.Scale.ToString(),
//                 NodeData.VisualData.DecalData.DecalSize
//             );
//         }
//         FString LogMessage = FString::Printf(
//             TEXT("%s | %s"),
//             *GeneralInfo,
//             *VisualDataLog
//         );
//         FResourceRouletteUtilityLog::Get().LogMessage(LogMessage, ELogLevel::Debug);
//     }
// }
