#include "ResourceRouletteManager.h"

#include "EngineUtils.h"
#include "ResourcePurityManager.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.h"
#include "ResourceNodeRandomizer.h"
#include "ResourceRouletteSubsystem.h"


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
	SeedManager = InSeedManager;
	ScanWorldResourceNodes(World);
	RandomizeWorldResourceNodes(World);
	SpawnWorldResourceNodes(World);
	UpdateWorldResourceNodes(World);
}


/// Manager method to collect resources list and purity list. Should be run 
/// only on initialization, although we may need to run it twice if some things
/// are initialized slowly
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
	InitMeshesToDestroy();
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		if (ResourceRouletteSubsystem->GetSessionAlreadySpawned() && !bIsResourcesScanned)
		{
			ResourceCollectionManager->SetCollectedResourcesNodes(
				ResourceRouletteSubsystem->GetSessionCollectedResourceNodes());
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
		ResourceCollectionManager->CollectWorldResources(World);
		ResourcePurityManager->CollectWorldPurities(World);
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
	if (bIsResourcesScanned && !bIsResourcesRandomized)
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
void UResourceRouletteManager::SpawnWorldResourceNodes(UWorld* World)
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("SpawnWorldResourceNodes aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}
	if (bIsResourcesScanned && bIsResourcesRandomized && !bIsResourcesSpawned)
	{
		ResourceNodeSpawner->SpawnWorldResources(World, ResourceNodeRandomizer);
		bIsResourcesSpawned = true;
		FResourceRouletteUtilityLog::Get().LogMessage("Resources Spawning completed successfully.", ELogLevel::Debug);
	}
	else
	{
		// FResourceRouletteUtilityLog::Get().LogMessage("Spawning Skipped.", ELogLevel::Debug);
	}
}

/// For now, this destroys any meshes that aren't explicity marked as our randomized meshes
/// This may not be strictly necessary, but requires more playtesting
/// @param World 
void UResourceRouletteManager::UpdateWorldResourceNodes(const UWorld* World) const
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("UpdateWorldResourceNodes aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}

	if (!(bIsResourcesScanned && bIsResourcesRandomized && bIsResourcesSpawned))
	{
		FResourceRouletteUtilityLog::Get().LogMessage("Updating Skipped.", ELogLevel::Debug);
		return;
	}

	for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
	{
		UStaticMeshComponent* StaticMeshComponent = *It;
		if (StaticMeshComponent && StaticMeshComponent->GetWorld() == World && !StaticMeshComponent->ComponentTags.
			Contains(ResourceRouletteTag))
		{
			if (UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh())
			{
				FName MeshPath = FName(*StaticMesh->GetPathName());

				if (MeshesToDestroy.Contains(MeshPath))
				{
					StaticMeshComponent->SetActive(false);
					StaticMeshComponent->SetVisibility(false);
					StaticMeshComponent->DestroyComponent();
				}
			}
		}
	}
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
