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
#include "Components/DecalComponent.h"
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
		bool bSpawned = false;

		if (NodeData.ResourceForm == EResourceForm::RF_LIQUID) // Decal-based nodes, but could include fracking oil nodes :(
		{
			bSpawned = SpawnResourceNodeDecal(World, NodeData, ResourceAssets);
		}
		else // It must be one of them Solid nodes
		{
			bSpawned = SpawnResourceNodeSolid(World, NodeData, ResourceAssets);
		}

		if (!bSpawned)
		{
			FResourceRouletteUtilityLog::Get().LogMessage(
				FString::Printf(TEXT("Failed to spawn resource node at location: %s"), *NodeData.Location.ToString()),
				ELogLevel::Warning);
		}
	}
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		ResourceRouletteSubsystem->SetSessionRandomizedResourceNodes(ProcessedNodes);
		ResourceRouletteSubsystem->SetSessionAlreadySpawned(true);
	}
}

/// Handles spawning nodes that use decals - in the case that's crude oil nodes
/// @param World World Context
/// @param NodeDataNode data to process
/// @param ResourceAssets Asset manager with our paths/decal size
/// @return True if succeeded
bool UResourceNodeSpawner::SpawnResourceNodeDecal(UWorld* World, FResourceNodeData& NodeData, const UResourceRouletteAssets* ResourceAssets)
{
    if (NodeData.ResourceClass.IsNone())
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            FString::Printf(TEXT("Invalid ResourceClass for node at location: %s"), *NodeData.Location.ToString()),
            ELogLevel::Warning);
        return false;
    }

    const FName ResourceClassName = NodeData.ResourceClass;
	
    TArray<FString> MaterialPaths = ResourceAssets->GetLiquidMaterials(ResourceClassName);
    float DecalScale = ResourceAssets->GetLiquidDecalScale(ResourceClassName);

    if (MaterialPaths.Num() == 0)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            FString::Printf(TEXT("No materials found for liquid resource: %s"), *ResourceClassName.ToString()),
            ELogLevel::Warning);
        return false;
    }
	
    UMaterialInterface* DecalMaterial = Cast<UMaterialInterface>(
        StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPaths[0]));

    if (!DecalMaterial)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            FString::Printf(TEXT("Failed to load decal material for resource: %s, Path: %s"), 
                            *ResourceClassName.ToString(), 
                            *MaterialPaths[0]),
            ELogLevel::Warning);
        return false;
    }
	
    UClass* Classname = FindObject<UClass>(ANY_PACKAGE, *NodeData.Classname);
    if (!Classname)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            FString::Printf(TEXT("Failed to find class for resource node: %s"), *NodeData.Classname),
            ELogLevel::Warning);
        return false;
    }

    AFGResourceNode* ResourceNode = World->SpawnActor<AFGResourceNode>(Classname, NodeData.Location, NodeData.Rotation);
    if (!ResourceNode)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            FString::Printf(TEXT("Failed to spawn resource node at location: %s"), *NodeData.Location.ToString()),
            ELogLevel::Warning);
        return false;
    }
	
    UDecalComponent* DecalComponent = NewObject<UDecalComponent>(ResourceNode);
    if (!DecalComponent)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            FString::Printf(TEXT("Failed to create DecalComponent for resource node at location: %s"), *NodeData.Location.ToString()),
            ELogLevel::Warning);
        ResourceNode->Destroy();
        return false;
    }

	DecalComponent->ComponentTags.Add(ResourceRouletteTag);
    DecalComponent->SetDecalMaterial(DecalMaterial);
    DecalComponent->DecalSize = FVector(80,DecalScale,DecalScale);
    DecalComponent->SetWorldLocation(NodeData.Location);
    // DecalComponent->SetWorldRotation(NodeData.Rotation);
	// DecalComponent->SetWorldRotation(FRotator::ZeroRotator);
	DecalComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
    DecalComponent->RegisterComponent();
	
    ResourceNode->SetRootComponent(DecalComponent);
    ResourceNode->AddInstanceComponent(DecalComponent);

	UClass* ResourceClass = FindObject<UClass>(ANY_PACKAGE, *NodeData.ResourceClass.ToString());
    ResourceNode->InitResource(ResourceClass, NodeData.Amount, NodeData.Purity);
    ResourceNode->SetActorScale3D(FVector(1.0f));
    ResourceNode->mResourceNodeType = NodeData.ResourceNodeType;
    ResourceNode->mCanPlacePortableMiner = NodeData.bCanPlaceResourceExtractor;
    ResourceNode->mCanPlaceResourceExtractor = NodeData.bCanPlaceResourceExtractor;
	
	if (!ResourceNode->mResourceNodeRepresentation)
	{
		ResourceNode->mResourceNodeRepresentation = NewObject<UFGResourceNodeRepresentation>(ResourceNode);
		ResourceNode->mResourceNodeRepresentation->SetupResourceNodeRepresentation(ResourceNode);
		
	}
	ResourceNode->UpdateNodeRepresentation();
	
	if (!ResourceNode->mBoxComponent)
	{
		ResourceNode->mBoxComponent = NewObject<UBoxComponent>(ResourceNode);
		ResourceNode->mBoxComponent->SetCollisionProfileName("Resource");
		ResourceNode->mBoxComponent->SetupAttachment(ResourceNode->GetRootComponent());
		ResourceNode->mBoxComponent->RegisterComponent();
	}

	ResourceNode->mBoxComponent->SetGenerateOverlapEvents(true);
	ResourceNode->mBoxComponent->SetWorldScale3D(FVector(30.0f, 30.0f, 2.0f));

	
    // Register the node in the tracking system
    NodeData.NodeGUID = FGuid::NewGuid();
    SpawnedResourceNodes.Add(NodeData.NodeGUID, ResourceNode);

    FResourceRouletteUtilityLog::Get().LogMessage(
        FString::Printf(TEXT("Successfully spawned decal resource node: %s at location: %s"),
                        *ResourceClassName.ToString(), *NodeData.Location.ToString()),
        ELogLevel::Debug);

    return true;
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

