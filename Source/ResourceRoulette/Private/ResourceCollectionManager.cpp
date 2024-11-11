#include "ResourceCollectionManager.h"
#include "Resources/FGResourceNode.h"
#include "ResourceRouletteUtility.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Resources/FGResourceDescriptor.h"
#include "Components/DecalComponent.h"


UResourceCollectionManager::UResourceCollectionManager()
{
	CollectedResourceNodes.Empty();
}

void UResourceCollectionManager::CollectWorldResources(const UWorld* World)
{
	if (!World) return;
	
	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
	{
		AFGResourceNode* ResourceNode = *It;

		if (!UResourceRouletteUtility::IsValidInfiniteResourceNode(ResourceNode))
		{
			continue;
		}
	}
}

	// switch (ResourceNode->GetResourceForm())
	// {
	// case EResourceForm::RF_SOLID:
	// 	CollectSolidResources(ResourceNode);
	// 	break;
	// case EResourceForm::RF_LIQUID:
	// 	CollectLiquidResources(ResourceNode);
	// 	break;
	// case EResourceForm::RF_HEAT:
	// 	CollectHeatResources(ResourceNode);
	// 	break;
	// case EResourceForm::RF_FRACKING:
	// 	CollectFrackingResources(ResourceNode);
	// 	break;
	// default:
	// 	break;
	// }

	
	
// void UResourceCollectionManager::DestroyMatchingMeshes(UWorld* World)
// {
// 	for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
// 	{
// 		UStaticMeshComponent* MeshComponent = *It;
// 		if (MeshComponent && MeshComponent->GetWorld() == World)
// 		{
// 			UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
// 			if (StaticMesh && ResourceAssets.Contains(StaticMesh->GetPathName()))
// 			{
// 				MeshComponent->SetActive(false);
// 				MeshComponent->SetVisibility(false);
// 				MeshComponent->DestroyComponent();
// 			}
// 		}
// 	}
// }

// /// Collects all the resources in the world and their information
// /// @param World World Context
// void UResourceCollectionManager::CollectWorldResources(const UWorld* World)
// {
// 	CollectedResourceNodes.Empty();
// 	FResourceRouletteUtilityLog::Get().LogMessage(TEXT("CollectWorldResources called"), ELogLevel::Debug);
// 	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
// 	{
// 		AFGResourceNode* ResourceNode = *It;
//
// 		if (!UResourceRouletteUtility::IsValidInfiniteResourceNode(ResourceNode))
// 		{
// 			continue;
// 		}
//
// 		// Collect node infos
// 		FResourceNodeData NodeData;
// 		NodeData.Location = ResourceNode->GetActorLocation();
// 		NodeData.Rotation = ResourceNode->GetActorRotation();
// 		NodeData.Scale = ResourceNode->GetActorScale3D();
// 		NodeData.Purity = ResourceNode->GetResoucePurity();
// 		NodeData.Amount = ResourceNode->GetResourceAmount();
// 		NodeData.ResourceClass = ResourceNode->GetResourceClass();
// 		NodeData.ResourceForm = ResourceNode->GetResourceForm();
// 		NodeData.bCanPlaceResourceExtractor = ResourceNode->CanPlaceResourceExtractor();
// 		NodeData.bIsOccupied = ResourceNode->IsOccupied();
// 		NodeData.VisualData.bIsUsingDecal = false;
// 		
// 		//Collect Mesh infos
// 		UStaticMesh* ResourceMesh = nullptr;
// 		bool bHasMeshActor = false;
// 		AActor* MeshActor = ResourceNode->GetMeshActor();
// 		if (MeshActor)
// 		{
// 			UStaticMeshComponent* MeshComponent = MeshActor->FindComponentByClass<UStaticMeshComponent>();
// 			if (MeshComponent && MeshComponent->GetStaticMesh())
// 			{
// 				ResourceMesh = MeshComponent->GetStaticMesh();
// 				NodeData.VisualData.MeshData.Mesh = ResourceMesh;
// 				NodeData.VisualData.MeshData.MeshPath = ResourceMesh->GetPathName();
// 				NodeData.VisualData.MeshData.Rotation = MeshComponent->GetComponentRotation();
// 				NodeData.VisualData.MeshData.Scale = MeshComponent->GetComponentScale();
// 				NodeData.VisualData.MeshData.Location = MeshComponent->GetComponentLocation();
// 				bHasMeshActor = true;
// 			}
// 		}
// 		
// 		if (!bHasMeshActor && NodeData.ResourceClass)
// 		{
// 			UFGResourceDescriptor* DescriptorCDO = Cast<UFGResourceDescriptor>(
// 				NodeData.ResourceClass->GetDefaultObject());
// 			if (DescriptorCDO && DescriptorCDO->mDepositMesh)
// 			{
// 				ResourceMesh = DescriptorCDO->mDepositMesh;
// 				NodeData.VisualData.MeshData.Mesh = ResourceMesh;
// 				NodeData.VisualData.MeshData.MeshPath = ResourceMesh->GetPathName();
// 				NodeData.VisualData.MeshData.Rotation = FRotator::ZeroRotator;
// 				NodeData.VisualData.MeshData.Scale = FVector(1.0f, 1.0f, 1.0f);
// 				NodeData.VisualData.MeshData.Location = FVector::ZeroVector;
// 			}
// 		}
// 		//Collect Material infos
// 		if (ResourceMesh)
// 		{
// 			int32 NumMaterials = ResourceMesh->GetStaticMaterials().Num();
// 			for (int32 i = 0; i < NumMaterials; i++)
// 			{
// 				UMaterialInterface* Material = ResourceMesh->GetMaterial(i);
// 				if (Material)
// 				{
// 					NodeData.VisualData.MeshData.Materials.Add(Material);
// 					NodeData.VisualData.MeshData.MaterialPaths.Add(Material->GetPathName());
// 				}
// 			}
// 		}
// 		else if (UDecalComponent* DecalComponent = ResourceNode->FindComponentByClass<UDecalComponent>())
// 		{
// 			NodeData.VisualData.bIsUsingDecal = true;
// 			NodeData.VisualData.DecalData.DecalMaterial = UFGResourceDescriptor::GetDecalMaterial(
// 				NodeData.ResourceClass);
// 			NodeData.VisualData.DecalData.DecalSize = UFGResourceDescriptor::GetDecalSize(NodeData.ResourceClass);
// 			NodeData.VisualData.DecalData.Rotation = DecalComponent->GetComponentRotation();
// 			NodeData.VisualData.DecalData.Scale = FVector(NodeData.VisualData.DecalData.DecalSize);
// 			NodeData.VisualData.DecalData.Location = DecalComponent->GetComponentLocation();
// 			NodeData.VisualData.DecalData.DecalMaterialPath = NodeData.VisualData.DecalData.DecalMaterial
// 				                                                  ? NodeData.VisualData.DecalData.DecalMaterial->
// 				                                                             GetPathName()
// 				                                                  : TEXT("No Decal");
// 			DecalComponent->DestroyComponent();
// 		}
// 		else
// 		{
// 			// If we didn't get either a material or a decal then we're missing something so try again...
// 			continue;
// 		}
// 		CollectedResourceNodes.Add(NodeData);
// 		ResourceNode->Destroy();
// 	}
// 	// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Total Collected Resource Nodes: %d"), CollectedResourceNodes.Num()), ELogLevel::Debug);
// 	LogCollectedResources();
// 	CollectedResourceNodes.Empty();
// }

// /// Logs all the collected resources to file so we can do some data analysis
// void UResourceCollectionManager::LogCollectedResources() const
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
