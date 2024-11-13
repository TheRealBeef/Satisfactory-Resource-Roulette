#include "ResourceNodeSpawner.h"
#include "ResourceNodeRandomizer.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Resources/FGResourceNode.h"
#include "Representation/FGResourceNodeRepresentation.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ResourceAssets.h"
#include "ResourceRouletteSubsystem.h"
#include "ResourceRouletteUtility.h"
#include "Kismet/GameplayStatics.h"

UResourceNodeSpawner::UResourceNodeSpawner()
{
	NodeRandomizer = nullptr;
}

void UResourceNodeSpawner::SpawnWorldResources(UWorld* World, UResourceNodeRandomizer* InNodeRandomizer)
{
	NodeRandomizer = InNodeRandomizer;
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("SpawnWorldResources aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}
	if (!NodeRandomizer)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("Failed to find UResourceNodeRandomizer instance.",
		                                              ELogLevel::Error);
		return;
	}

	const TArray<FResourceNodeData>& ProcessedNodes = NodeRandomizer->GetProcessedNodes();
	UResourceRouletteAssets* ResourceAssets = NewObject<UResourceRouletteAssets>();

	FResourceRouletteUtilityLog::Get().LogMessage("Attempting to spawn resource node.", ELogLevel::Debug);
	FResourceRouletteUtilityLog::Get().LogMessage(
		FString::Printf(TEXT("Number of nodes to spawn: %d"), ProcessedNodes.Num()), ELogLevel::Debug);

	for (const FResourceNodeData& NodeData : ProcessedNodes)
	{
		if (!SpawnCustomResourceNodeSolid(World, NodeData, ResourceAssets))
		{
			FResourceRouletteUtilityLog::Get().LogMessage(
				FString::Printf(TEXT("Failed to spawn resource node at location: %s"), *NodeData.Location.ToString()),
				ELogLevel::Warning
			);
		}
	}
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		ResourceRouletteSubsystem->SetSessionAlreadySpawned(true);
	}
}

bool UResourceNodeSpawner::SpawnCustomResourceNodeSolid(UWorld* World, const FResourceNodeData& NodeData,
                                                        const UResourceRouletteAssets* ResourceAssets)
{
	if (!NodeData.ResourceClass)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("Invalid ResourceClass for node at location: %s"), *NodeData.Location.ToString()),
			ELogLevel::Warning
		);
		return false;
	}

	// For now only Solid Nodes TODO Add other node types
	const FName ResourceClassName = NodeData.ResourceClass->GetFName();
	const FString MeshPath = ResourceAssets->GetSolidMesh(ResourceClassName);
	TArray<FString> MaterialPaths = ResourceAssets->GetSolidMaterial(ResourceClassName);
	const FVector Offset = ResourceAssets->GetSolidOffset(ResourceClassName);
	const FVector Scale = ResourceAssets->GetSolidScale(ResourceClassName);

	// Load the mesh asset
	UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
	if (!Mesh)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("Failed to load mesh for resource: %s"), *ResourceClassName.ToString()),
			ELogLevel::Warning
		);
		return false;
	}

	// Load materials
	TArray<UMaterialInterface*> Materials;
	for (const FString& MaterialPath : MaterialPaths)
	{
		if (UMaterialInterface* Material = Cast<UMaterialInterface>(
			StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath)))
		{
			Materials.Add(Material);
		}
		else
		{
			FResourceRouletteUtilityLog::Get().LogMessage(
				FString::Printf(
					TEXT("Failed to load material for resource: %s, MaterialPath: %s"), *ResourceClassName.ToString(),
					*MaterialPath),
				ELogLevel::Warning);
			return false;
		}
	}

	AFGResourceNode* ResourceNode = World->SpawnActor<AFGResourceNode>(NodeData.Classname, NodeData.Location,
	                                                                   NodeData.Rotation);
	if (!ResourceNode)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("Failed to spawn AFGResourceNode at location: %s"), *NodeData.Location.ToString()),
			ELogLevel::Warning);
		return false;
	}
	// FResourceRouletteUtilityLog::Get().LogMessage(
	//     FString::Printf(TEXT("Spawned Resource Node at location: %s"), *NodeData.Location.ToString()), ELogLevel::Debug);


	ResourceNode->InitResource(NodeData.ResourceClass, NodeData.Amount, NodeData.Purity);
	ResourceNode->SetActorScale3D(NodeData.Scale);
	ResourceNode->mResourceNodeType = NodeData.ResourceNodeType;
	ResourceNode->mCanPlacePortableMiner = NodeData.bCanPlaceResourceExtractor;
	ResourceNode->mCanPlaceResourceExtractor = NodeData.bCanPlaceResourceExtractor;

	if (!ResourceNode->mResourceNodeRepresentation)
	{
		ResourceNode->mResourceNodeRepresentation = NewObject<UFGResourceNodeRepresentation>(ResourceNode);
		ResourceNode->mResourceNodeRepresentation->SetupResourceNodeRepresentation(ResourceNode);
	}

	ResourceNode->InitRadioactivity();
	ResourceNode->UpdateRadioactivity();

	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(ResourceNode);
	if (!MeshComponent)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(
				TEXT("Failed to spawn MeshComponent for resource node at location: %s"), *NodeData.Location.ToString()),
			ELogLevel::Warning);
		return false;
	}

	MeshComponent->SetCollisionProfileName("ResourceMesh");
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetVisibility(true);
	MeshComponent->ComponentTags.Add(ResourceRouletteTag);

	for (int32 i = 0; i < Materials.Num(); ++i)
	{
		MeshComponent->SetMaterial(i, Materials[i]);
	}

	ResourceNode->SetRootComponent(MeshComponent);
	MeshComponent->RegisterComponent();

	FVector Min, Max;
	MeshComponent->GetLocalBounds(Min, Max);
	const FVector MeshPivot = (Min + Max) / 2.0f;
	FTransform MeshTransform = MeshComponent->GetComponentTransform();
	FVector AdjustedPivot = MeshTransform.TransformPosition(MeshPivot);
	const FVector CorrectedLocation = NodeData.Location - (AdjustedPivot - MeshTransform.GetLocation()) + Offset;
	MeshComponent->SetWorldLocation(CorrectedLocation);
	// TODO resolve the rotation stuff, maybe it's better to go back to actor and raycast and set up "nice" node locations?
	// MeshComponent->SetWorldRotation(NodeData.Rotation);
	MeshComponent->SetWorldScale3D(Scale);

	// FResourceRouletteUtilityLog::Get().LogMessage(
	//     FString::Printf(TEXT("Spawned Mesh for Node at location: %s"), *NodeData.Location.ToString()), ELogLevel::Debug);

	return true;
}
