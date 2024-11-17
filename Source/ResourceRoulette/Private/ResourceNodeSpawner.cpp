#include "ResourceNodeSpawner.h"
#include "ResourceNodeRandomizer.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
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
	// SpawnedResourceNodes.Empty();
}

/// Parent method to Spawn world resources
/// TODO Currently only spawns solid resources, will add additional resources soon(TM)
/// @param World World Context
/// @param InNodeRandomizer The node randomizer instance so we can grab nodes from it
/// @param IsFromSaved If we have already randomized nodes, we should load from save instead
void UResourceNodeSpawner::SpawnWorldResources(UWorld* World, UResourceNodeRandomizer* InNodeRandomizer, const bool IsFromSaved)
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
	ProcessedNodes.Empty();
	if (IsFromSaved)
	{
		AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World);
		ProcessedNodes = ResourceRouletteSubsystem->GetSessionRandomizedResourceNodes();
	}
	else
	{
		ProcessedNodes = NodeRandomizer->GetProcessedNodes();
	}
	const UResourceRouletteAssets* ResourceAssets = NewObject<UResourceRouletteAssets>();
	
	FResourceRouletteUtilityLog::Get().LogMessage(
		FString::Printf(TEXT("Number of nodes to spawn: %d"), ProcessedNodes.Num()), ELogLevel::Debug);

	for (FResourceNodeData& NodeData : ProcessedNodes)
	{
		if (!SpawnResourceNodeSolid(World, NodeData, ResourceAssets))
		{
			FResourceRouletteUtilityLog::Get().LogMessage(
				FString::Printf(TEXT("Failed to spawn resource node at location: %s"), *NodeData.Location.ToString()),
				ELogLevel::Warning
			);
		}
	}
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		ResourceRouletteSubsystem->SetSessionRandomizedResourceNodes(ProcessedNodes);
		ResourceRouletteSubsystem->SetSessionAlreadySpawned(true);
	}
}

/// Spawns Solid resource nodes into the world
/// Thanks to Oukibt https://github.com/oukibt/ResourceNodeRandomizer 
/// I relied heavily on their original SolidResourceNodeSpawner.cpp to know what needed to be set in order to spawn nodes
/// @param World World Context
/// @param NodeData The node data for what needs spawned
/// @param ResourceAssets Hardcoded :( list of assets
/// @return Returns True if succeeds
bool UResourceNodeSpawner::SpawnResourceNodeSolid(UWorld* World, FResourceNodeData& NodeData,
                                                        const UResourceRouletteAssets* ResourceAssets)
{
	if (NodeData.ResourceClass.IsNone())
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("Invalid ResourceClass for node at location: %s"), *NodeData.Location.ToString()),
			ELogLevel::Warning
		);
		return false;
	}

	// For now only Solid Nodes TODO Add other node types
	const FName ResourceClassName = NodeData.ResourceClass;
	const FString MeshPath = ResourceAssets->GetSolidMesh(ResourceClassName);
	TArray<FString> MaterialPaths = ResourceAssets->GetSolidMaterial(ResourceClassName);
	NodeData.Offset = ResourceAssets->GetSolidOffset(ResourceClassName);
	NodeData.Scale = ResourceAssets->GetSolidScale(ResourceClassName);

	
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
	
	UClass* Classname = FindObject<UClass>(ANY_PACKAGE, *NodeData.Classname);
	// TODO - Probably we should do something about the rotation

	AFGResourceNode* ResourceNode;
	if (NodeData.IsRayCasted)
	{
		ResourceNode = World->SpawnActor<AFGResourceNode>(Classname, NodeData.Location,NodeData.Rotation);
	}
	else
	{

		ResourceNode = World->SpawnActor<AFGResourceNode>(Classname, NodeData.Location,FRotator::ZeroRotator);
	}
	if (!ResourceNode)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("Failed to spawn AFGResourceNode at location: %s"), *NodeData.Location.ToString()),
			ELogLevel::Warning);
		return false;
	}
	// NodeData.NodePointer = ResourceNode;
	
	// FResourceRouletteUtilityLog::Get().LogMessage(
	//     FString::Printf(TEXT("Spawned Resource Node at location: %s"), *NodeData.Location.ToString()), ELogLevel::Debug);

	UClass* ResourceClass = FindObject<UClass>(ANY_PACKAGE, *NodeData.ResourceClass.ToString());
	ResourceNode->InitResource(ResourceClass, NodeData.Amount, NodeData.Purity);
	ResourceNode->SetActorScale3D(NodeData.Scale);
	ResourceNode->mResourceNodeType = NodeData.ResourceNodeType;
	ResourceNode->mCanPlacePortableMiner = NodeData.bCanPlaceResourceExtractor;
	ResourceNode->mCanPlaceResourceExtractor = NodeData.bCanPlaceResourceExtractor;

	if (!ResourceNode->mResourceNodeRepresentation)
	{
		ResourceNode->mResourceNodeRepresentation = NewObject<UFGResourceNodeRepresentation>(ResourceNode);
		ResourceNode->mResourceNodeRepresentation->SetupResourceNodeRepresentation(ResourceNode);
	}
	
	ResourceNode->UpdateMeshFromDescriptor();
	ResourceNode->UpdateNodeRepresentation();
	
	ResourceNode->InitRadioactivity();

	ResourceNode->mBoxComponent->SetWorldLocation(NodeData.Location);
	ResourceNode->mBoxComponent->SetWorldScale3D(FVector(30.0f, 30.0f, 2.0f));
	ResourceNode->mBoxComponent->SetCollisionProfileName("Resource");

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

	// FVector Min, Max;
	// MeshComponent->GetLocalBounds(Min, Max);
	// const FVector MeshPivot = (Min + Max) / 2.0f;
	// FTransform MeshTransform = MeshComponent->GetComponentTransform();
	// FVector AdjustedPivot = MeshTransform.TransformPosition(MeshPivot);
	// FVector CorrectedLocation = NodeData.Location - (AdjustedPivot - MeshTransform.GetLocation()) + Offset;
	FVector CorrectedLocation = NodeData.Location + NodeData.Offset;
	MeshComponent->SetWorldLocation(CorrectedLocation);
	// TODO resolve the rotation stuff, maybe it's better to go back to actor and raycast and set up "nice" node locations?
	MeshComponent->SetWorldScale3D(NodeData.Scale);
	if (NodeData.IsRayCasted)
	{
		MeshComponent->SetWorldRotation(NodeData.Rotation);
	}
	else
	{
		MeshComponent->SetWorldRotation(FRotator::ZeroRotator);
	}

	// FResourceRouletteUtilityLog::Get().LogMessage(
	//     FString::Printf(TEXT("Spawned Mesh for Node at location: %s"), *NodeData.Location.ToString()), ELogLevel::Debug);

	NodeData.NodeGUID = FGuid::NewGuid();
	SpawnedResourceNodes.Add(NodeData.NodeGUID, ResourceNode);
	
	return true;
}

