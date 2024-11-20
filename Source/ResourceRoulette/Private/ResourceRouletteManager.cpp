#include "ResourceRouletteManager.h"

#include "EngineUtils.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.h"
#include "ResourceNodeRandomizer.h"
#include "ResourceRouletteSubsystem.h"
#include "Components/DecalComponent.h"
#include "Equipment/FGResourceScanner.h"
#include "Kismet/GameplayStatics.h"
#include "ResourceRouletteConfigStruct.h"


/// We may want to add a check rather than just all 4 managers, as we aren't explicity removing these on reload
/// Alternatively we could ensure they're destroyed (we need destructor method)
UResourceRouletteManager::UResourceRouletteManager()
{
	ResourceCollectionManager = CreateDefaultSubobject<UResourceCollectionManager>(TEXT("ResourceCollectionManager"));
	ResourcePurityManager = CreateDefaultSubobject<UResourcePurityManager>(TEXT("ResourcePurityManager"));
	ResourceNodeRandomizer = CreateDefaultSubobject<UResourceNodeRandomizer>("ResourceNodeRandomizer");
	ResourceNodeSpawner = CreateDefaultSubobject<UResourceNodeSpawner>("ResourceNodeSpawner");
	SeedManager = nullptr;
	bIsResourcesScanned = false;
	bIsResourcesRandomized = false;
	bIsResourcesSpawned = false;
	FResourceRouletteUtilityLog::Get().LogMessage("Resource Manager initialized successfully.", ELogLevel::Debug);
}

/// Update method that's called to process the world
/// @param World - World context
/// @param InSeedManager - Seed manager instance so we can propogate these values
void UResourceRouletteManager::Update(UWorld* World, AResourceRouletteSeedManager* InSeedManager)
{
	const AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World);
	SeedManager = InSeedManager;
	ScanWorldResourceNodes(World);
	RandomizeWorldResourceNodes(World);
	SpawnWorldResourceNodes(World, ResourceRouletteSubsystem->GetSessionAlreadySpawned());
	UpdateWorldResourceNodes(World);
}


/// Manager method to collect resources list and purity list. Should be run 
/// only on initialization
/// @param World World Context
void UResourceRouletteManager::ScanWorldResourceNodes(const UWorld* World)
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("ScanWorldResourceNodes aborted: World is invalid.",
													  ELogLevel::Error);
		return;
	}
	if (!ResourceCollectionManager || !ResourcePurityManager)
	{
		return;
	}
	FResourceRouletteConfigStruct config = FResourceRouletteConfigStruct::GetActiveConfig(ResourceCollectionManager);
	UResourceRouletteUtility::UpdateValidResourceClasses(config);
	UResourceRouletteUtility::UpdateNonGroupableResources(config);
	
	
	InitMeshesToDestroy();
		
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		if (ResourceRouletteSubsystem->GetSessionAlreadySpawned() && !bIsResourcesScanned)
		{
			ResourceCollectionManager->SetCollectedResourcesNodes(ResourceRouletteSubsystem->GetSessionRandomizedResourceNodes());
			// TODO Add purity manager stuff too

			// Destroy the vanilla actors!
			for (TActorIterator<AFGResourceNode> It(World); It; ++It)
			{
				AFGResourceNode* ResourceNode = *It;

				
				if (!UResourceRouletteUtility::IsValidInfiniteResourceNode(ResourceNode))
				{
					continue;
				}
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
				ResourceNode->Destroy();
			}
			bIsResourcesScanned = true;
		}
	}
	if (!bIsResourcesScanned)
	{
		ResourcePurityManager->CollectWorldPurities(World);
		ResourceCollectionManager->CollectWorldResources(World);
		bIsResourcesScanned = true;
		FResourceRouletteUtilityLog::Get().LogMessage("Resource Scan completed successfully.", ELogLevel::Debug);
	}
	else
	{
		// FResourceRouletteUtilityLog::Get().LogMessage("New Resource scan not needed.", ELogLevel::Debug);
	}
}

/// Manager method to randomize world resources by updating the struct that holds all world resources
/// @param World 
void UResourceRouletteManager::RandomizeWorldResourceNodes(UWorld* World)
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("RandomizeWorldResourceNodes aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}

	
	// Set Grouping Radius
	FResourceRouletteConfigStruct config = FResourceRouletteConfigStruct::GetActiveConfig(ResourceCollectionManager);
	ResourceNodeRandomizer->SetGroupingRadius(config.GroupingOptions.GroupRadius);
	
	if (const AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		if (ResourceRouletteSubsystem->GetSessionAlreadySpawned())
		{
			bIsResourcesRandomized = true;
		}
	}
	if (bIsResourcesScanned && !bIsResourcesRandomized )
	{
		ResourceNodeRandomizer->RandomizeWorldResources(World, ResourceCollectionManager, ResourcePurityManager,
		                                                SeedManager);
		bIsResourcesRandomized = true;
		FResourceRouletteUtilityLog::Get().LogMessage("Resource Randomization completed successfully.",
		                                              ELogLevel::Debug);
	}
	else
	{
		// FResourceRouletteUtilityLog::Get().LogMessage("Randomization Skipped.", ELogLevel::Debug);
	}
}

/// Manager method to spawn all the resource nodes needed
/// @param World 
void UResourceRouletteManager::SpawnWorldResourceNodes(UWorld* World, bool IsFromSaved)
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("SpawnWorldResourceNodes aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}
	if (bIsResourcesScanned && bIsResourcesRandomized && !bIsResourcesSpawned)
	{
		ResourceNodeSpawner->SpawnWorldResources(World, ResourceNodeRandomizer, IsFromSaved);
		if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
		{
			const TArray<FResourceNodeData>& ProcessedNodes = ResourceRouletteSubsystem->GetSessionRandomizedResourceNodes();
			UResourceRouletteUtility::AssociateExtractorsWithNodes(World, ProcessedNodes, ResourceNodeSpawner->GetSpawnedResourceNodes());
		}

		AFGResourceScanner* ResourceScanner = Cast<AFGResourceScanner>(
	UGameplayStatics::GetActorOfClass(World, AFGResourceScanner::StaticClass()));
		if (ResourceScanner)
		{
			ResourceScanner->GenerateNodeClusters();
			FResourceRouletteUtilityLog::Get().LogMessage("Node Clusters generated successfully.", ELogLevel::Debug);
		}
		
		bIsResourcesSpawned = true;
		FResourceRouletteUtilityLog::Get().LogMessage("Resources Spawning completed successfully.", ELogLevel::Debug);
	}
	else
	{
		// FResourceRouletteUtilityLog::Get().LogMessage("Spawning Skipped.", ELogLevel::Debug);
	}
}

/// For now, this destroys any meshes that aren't explicity marked as our randomized meshes
/// It also updates the locations of resource nodes based on raycasting if they haven't
/// been raycast before
/// Destroying any vanilla meshes on udpate may not be necessary, but requires more playtesting
/// @param World 
void UResourceRouletteManager::UpdateWorldResourceNodes(const UWorld* World) const
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("UpdateWorldResourceNodes aborted: World is invalid.",ELogLevel::Error);
		return;
	}

	if (!(bIsResourcesScanned && bIsResourcesRandomized && bIsResourcesSpawned))
	{
		FResourceRouletteUtilityLog::Get().LogMessage("Updating Skipped.", ELogLevel::Debug);
		return;
	}

	// double StartTotalTime = FPlatformTime::Seconds();
	
	FVector PlayerLocation;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (PlayerController && PlayerController->GetPawn())
	{
		PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
	}
	else
	{
		return;
	}

	// Search 250m (about 31 foundations) around player to update nodes
	// TODO Need to test on lower graphical settings to see if this fails
	// Maybe it needs to be reduced based on graphics values?
	const float UpdateRadius = 25000.0f;

	AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World);
	if (!ResourceRouletteSubsystem)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("ResourceRouletteSubsystem is invalid.", ELogLevel::Error);
		return;
	}
	
	TArray<FResourceNodeData> ProcessedNodes = ResourceRouletteSubsystem->GetSessionRandomizedResourceNodes();

	
	// double StartNodeUpdatingTime = FPlatformTime::Seconds();

	// Somehow this takes <1ms to run normally, even when we're updating and raycasting things
	// I have no idea how, but this is some dark magic UE must be running behind the scenes
	bool bNodeUpdated = false;
	for (FResourceNodeData& NodeData : ProcessedNodes)
	{
		if (NodeData.ResourceForm == EResourceForm::RF_LIQUID)
		{
			// We shouldn't really raycast oil nodes since they're decals...
			continue;
		}
		if (!NodeData.IsRayCasted && FVector::Dist(NodeData.Location, PlayerLocation) <= UpdateRadius)
		{
			if (AFGResourceNode** ResourceNodePtr = ResourceNodeSpawner->GetSpawnedResourceNodes().Find(NodeData.NodeGUID))
			{
				AFGResourceNode* ResourceNode = *ResourceNodePtr;
				if (UResourceRouletteUtility::CalculateLocationAndRotationForNode(NodeData, World, ResourceNode))
				{
					bNodeUpdated = true;
					ResourceNode->SetActorRotation(NodeData.Rotation, ETeleportType::TeleportPhysics);
			
					if (UStaticMeshComponent* MeshComponent = ResourceNode->FindComponentByClass<UStaticMeshComponent>())
					{
						// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Updating MeshComponent Location to: %s, Rotation to: %s"),
						// 	*NodeData.Location.ToString(), *NodeData.Rotation.ToString()),	ELogLevel::Debug);
						FVector CorrectedLocation = NodeData.Location + NodeData.Offset;
						MeshComponent->SetWorldLocation(CorrectedLocation,false,nullptr,ETeleportType::TeleportPhysics);
						// MeshComponent->SetWorldLocation(NodeData.Location, false, nullptr, ETeleportType::TeleportPhysics);
						MeshComponent->SetWorldRotation(NodeData.Rotation, false, nullptr, ETeleportType::TeleportPhysics);

					}
				}
			}
		}
	}
		
	if (bNodeUpdated)
	{
		ResourceRouletteSubsystem->SetSessionRandomizedResourceNodes(ProcessedNodes);
	}

	// double NodeUpdatingTime = (FPlatformTime::Seconds() - StartNodeUpdatingTime)*1000.0f;
	// double StartMeshDestroyingTime = FPlatformTime::Seconds();

	// This runs significantly faster
	TArray<UStaticMeshComponent*> WorldMeshComponents;
	for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
	{
		UStaticMeshComponent* StaticMeshComponent = *It;
		if (StaticMeshComponent && StaticMeshComponent->GetWorld() == World)
		{
			WorldMeshComponents.Add(StaticMeshComponent);
		}
	}

	TArray<UStaticMeshComponent*> ComponentsToDestroy;
	
	ParallelFor(WorldMeshComponents.Num(), [&](int32 Index)
	{
		UStaticMeshComponent* StaticMeshComponent = WorldMeshComponents[Index];
		if (StaticMeshComponent && !StaticMeshComponent->ComponentTags.Contains(ResourceRouletteTag))
		{
			if (const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh())
			{
				FName MeshPath = FName(*StaticMesh->GetPathName());
				if (MeshesToDestroy.Contains(MeshPath))
				{
					FScopeLock Lock(&CriticalSection);
					ComponentsToDestroy.Add(StaticMeshComponent);
				}
			}
		}
	});
	
	for (UStaticMeshComponent* StaticMeshComponent : ComponentsToDestroy)
	{
		StaticMeshComponent->SetActive(false);
		StaticMeshComponent->SetVisibility(false);
		StaticMeshComponent->DestroyComponent();
	}

	// TODO Check to see how many elements this is so we can determine if it needs parallelized too (probably not)
	TArray<UDecalComponent*> DecalComponents;
	for (UDecalComponent* DecalComponent : DecalComponents)
	{
		if (DecalComponent && !DecalComponent->ComponentTags.Contains(ResourceRouletteTag))
		{
			DecalComponent->SetVisibility(false);
			DecalComponent->DestroyComponent();
		}
	}
	
	// double MeshDestroyingTime = (FPlatformTime::Seconds() - StartMeshDestroyingTime)*1000.0f;
	// double TotalTime = (FPlatformTime::Seconds() - StartTotalTime)*1000.0f;
	// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Node updating took: %f ms"), NodeUpdatingTime), ELogLevel::Debug);
	// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Mesh destroying took: %f ms"), MeshDestroyingTime), ELogLevel::Debug);
	// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Total execution time: %f ms"), TotalTime), ELogLevel::Debug);
}

/// Creates the lsit of meshes to destroy in a faster/smaller array to see if it improves speed
void UResourceRouletteManager::InitMeshesToDestroy()
{
	MeshesToDestroy.Empty();

	// Add solid resource paths
	const TMap<FName, FResourceRouletteAssetSolid>& SolidResourceInfoMap =
		UResourceRouletteAssets::SolidResourceInfoMap;
	for (const auto& Pair : SolidResourceInfoMap)
	{
		MeshesToDestroy.Add(FName(*Pair.Value.MeshPath));
	}

	// Add fracking resource paths
	// const TMap<FName, FResourceRouletteAssetFracking>& FrackingResourceInfoMap = UResourceRouletteAssets::FrackingResourceInfoMap;
	// for (const auto& Pair : FrackingResourceInfoMap)
	// {
	// 	if (Pair.Value.MeshPaths.Num() > 0)
	// 	{
	// 		MeshesToDestroy.Add(FName(*Pair.Value.MeshPaths[0]));
	// 	}
	// }
}


#define LOCTEXT_NAMESPACE "FResourceRouletteModule"

void FResourceRouletteModule::StartupModule()
{
}

void FResourceRouletteModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FResourceRouletteModule, ResourceRoulette)
