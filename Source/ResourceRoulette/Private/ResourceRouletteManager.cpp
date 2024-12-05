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
#include "ResourceRouletteCompatibilityManager.h"
#include "Components/BoxComponent.h"
#include "SessionSettings/SessionSettingsManager.h"


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
/// @param bReroll - Called when re-rolling resources
void UResourceRouletteManager::Update(UWorld* World, AResourceRouletteSeedManager* InSeedManager, bool bReroll)
{
	const AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World);
	if (bReroll)
	{
		bIsResourcesScanned = false;
		bIsResourcesRandomized = false;
		bIsResourcesSpawned = false;
	}
	SeedManager = InSeedManager;
	ScanWorldResourceNodes(World, bReroll);
	RandomizeWorldResourceNodes(World, bReroll);
	// Only pass true as parameter if not reroll & we didn't already have saved, otherwise pass false
	SpawnWorldResourceNodes(World, (!bReroll && ResourceRouletteSubsystem->GetSessionAlreadySpawned()));
	UpdateWorldResourceNodes(World);
}

/// Manager method to collect resources list and purity list. Should be run 
/// only on initialization
/// @param World World Context
/// @param bReroll
void UResourceRouletteManager::ScanWorldResourceNodes(UWorld* World, bool bReroll)
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
	if (!bIsResourcesScanned)
	{
		USessionSettingsManager* SessionSettings = GetWorld()->GetSubsystem<USessionSettingsManager>();
		UResourceRouletteUtility::UpdateValidResourceClasses(SessionSettings);
		UResourceRouletteUtility::UpdateNonGroupableResources(SessionSettings);
	}
	// Don't repeat this on reroll
	if (!bReroll && !bIsResourcesScanned)
	{
		InitMeshesToDestroy();
        	
		// Here we can add classnames/tags for mods we want to have compatible with our mod
		// Buildable Resource Nodes Redux
		ResourceRouletteCompatibilityManager::RegisterResourceClass("BRN_Base_ResourceNode_C", "BuildableResourceNodeReduxObject");
		ResourceRouletteCompatibilityManager::RegisterResourceClass("BRN_Build_Base_ResNode_C", "BuildableResourceNodeReduxObject");
		ResourceRouletteCompatibilityManager::RegisterResourceClass("BuildEffect_Default_C", "BuildEffect_Default_C");

		// Resource Node Creator
		// ResourceRouletteCompatibilityManager::RegisterResourceClass("BP_ResourceNode_C", "FGBuildable");
		ResourceRouletteCompatibilityManager::RegisterResourceClass("FGBuildable", "FGBuildable");

		// General Holograms
		ResourceRouletteCompatibilityManager::RegisterResourceClass("FGBuildableHologram", "FGBuildable");

		RegisteredTags = ResourceRouletteCompatibilityManager::GetRegisteredTags();
		ResourceRouletteCompatibilityManager::TagExistingActors(World);
		ResourceRouletteCompatibilityManager::SetupActorSpawnCallback(World);
	}
	
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		// If we have previously randomized nodes in this save and now we should re-roll
		if (ResourceRouletteSubsystem->GetSessionAlreadySpawned() && bReroll && !bIsResourcesScanned)
		{
			// ResourceCollectionManager->SetCollectedResourcesNodes(ResourceRouletteSubsystem->GetOriginalResourceNodes());
			// Here reset the raycast for all the nodes before passing it, to ensure that we resettle them properly.
			TArray<FResourceNodeData> OriginalNodes = ResourceRouletteSubsystem->GetOriginalResourceNodes();
			for (FResourceNodeData& NodeData : OriginalNodes)
			{
				NodeData.IsRayCasted = false;
			}
			ResourceCollectionManager->SetCollectedResourcesNodes(OriginalNodes);
			ResourcePurityManager->CollectOriginalPurities(OriginalNodes);
			// Destroy both vanilla actors and our own nodes!
			for (TActorIterator<AFGResourceNode> It(World); It; ++It)
			{
				AFGResourceNode* ResourceNode = *It;

				// Skip any of the compatibility tagged ones
				bool bIsTagged = false;
				for (const FName& RegisteredTag : RegisteredTags)
				{
					if (ResourceNode->Tags.Contains(RegisteredTag))
					{
						bIsTagged = true;
						break;
					}
				}
				if (bIsTagged)
				{
					continue;
				}
				
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
			FResourceRouletteUtilityLog::Get().LogMessage("Resource Scan from Reroll.", ELogLevel::Debug);
		}
		// If we have previously randomized nodes in this save
		if (ResourceRouletteSubsystem->GetSessionAlreadySpawned() && !bReroll && !bIsResourcesScanned)
		{
			ResourceCollectionManager->SetCollectedResourcesNodes(ResourceRouletteSubsystem->GetSessionRandomizedResourceNodes());
			// Destroy the actors!
			for (TActorIterator<AFGResourceNode> It(World); It; ++It)
			{
				AFGResourceNode* ResourceNode = *It;

				// Skip any of the compatibility tagged ones
				bool bIsTagged = false;
				for (const FName& RegisteredTag : RegisteredTags)
				{
					if (ResourceNode->Tags.Contains(RegisteredTag))
					{
						bIsTagged = true;
						break;
					}
				}
				if (bIsTagged)
				{
					continue;
				}
				
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
			FResourceRouletteUtilityLog::Get().LogMessage("Resource Scan from Save.", ELogLevel::Debug);
		}
		// If we haven't previously randomized nodes in this save
		if (!bIsResourcesScanned)
		{
			ResourcePurityManager->CollectWorldPurities(World);
			ResourceCollectionManager->CollectWorldResources(World);
			ResourceRouletteSubsystem->SetOriginalResourceNodes(ResourceCollectionManager->GetCollectedResourceNodes());
			bIsResourcesScanned = true;
			FResourceRouletteUtilityLog::Get().LogMessage("Resource Scan completed successfully.", ELogLevel::Debug);
		}
		else
		{
			// FResourceRouletteUtilityLog::Get().LogMessage("New Resource scan not needed.", ELogLevel::Debug);
		}
	}
}

/// Manager method to randomize world resources by updating the struct that holds all world resources
/// @param World 
void UResourceRouletteManager::RandomizeWorldResourceNodes(UWorld* World, bool bReroll)
{
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("RandomizeWorldResourceNodes aborted: World is invalid.",
		                                              ELogLevel::Error);
		return;
	}

	
	// Set Grouping Radius
	USessionSettingsManager* SessionSettings = GetWorld()->GetSubsystem<USessionSettingsManager>();
	ResourceNodeRandomizer->SetGroupingRadius(SessionSettings->GetFloatOptionValue("ResourceRoulette.GroupOpt.GroupRadius"));
	
	if (const AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(World))
	{
		if (ResourceRouletteSubsystem->GetSessionAlreadySpawned() && !bReroll)
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
	// TODO: Need to test on lower graphical settings to see if this fails
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
					// ResourceNode->SetActorLocation(NodeData.Location,false, nullptr, ETeleportType::TeleportPhysics);
					// ResourceNode->SetActorRotation(NodeData.Rotation, ETeleportType::TeleportPhysics);
			
					if (UStaticMeshComponent* MeshComponent = ResourceNode->FindComponentByClass<UStaticMeshComponent>())
					{
						// FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Updating MeshComponent Location to: %s, Rotation to: %s"),
						// 	*NodeData.Location.ToString(), *NodeData.Rotation.ToString()),	ELogLevel::Debug);
						FVector CorrectedLocation = NodeData.Location + NodeData.Offset;
						MeshComponent->SetWorldLocation(CorrectedLocation,false,nullptr,ETeleportType::TeleportPhysics);
						// MeshComponent->SetWorldLocation(NodeData.Location, false, nullptr, ETeleportType::TeleportPhysics);
						MeshComponent->SetWorldRotation(NodeData.Rotation, false, nullptr, ETeleportType::TeleportPhysics);

					}
					if (UBoxComponent* CollisionBox = ResourceNode->FindComponentByClass<UBoxComponent>())
					{
						CollisionBox->SetWorldLocation(NodeData.Location, false, nullptr, ETeleportType::TeleportPhysics);
						CollisionBox->SetWorldRotation(NodeData.Rotation, false, nullptr, ETeleportType::TeleportPhysics);
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
		if (StaticMeshComponent)
		{
			for (const FName& Tag : StaticMeshComponent->ComponentTags)
			{
				if (RegisteredTags.Contains(Tag) || Tag == ResourceRouletteTag)
				{
					return;
				}
			}
			
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
		// if (AActor* OwnerActor = StaticMeshComponent->GetAttachmentRootActor())
		// {
		// 	FString ClassHierarchy = OwnerActor->GetClass()->GetName();
		// 	UClass* CurrentClass = OwnerActor->GetClass()->GetSuperClass();
		// 	while (CurrentClass)
		// 	{
		// 		ClassHierarchy += FString(TEXT(" -> ")) + CurrentClass->GetName();
		// 		CurrentClass = CurrentClass->GetSuperClass();
		// 	}
		// 	FResourceRouletteUtilityLog::Get().LogMessage(
		// 		FString::Printf(
		// 			TEXT("Destroying mesh %s, class heirarchy is: %s"), 
		// 			*StaticMeshComponent->GetName(), *ClassHierarchy),
		// 		ELogLevel::Debug
		// 	);
		// }
		StaticMeshComponent->SetActive(false);
		StaticMeshComponent->SetVisibility(false);
		StaticMeshComponent->DestroyComponent();
	}
	
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

/// Prep to remove the mod and destroy extractors in the world
void UResourceRouletteManager::PrepModForRemoval()
{
	
	if (AResourceRouletteSubsystem* ResourceRouletteSubsystem = AResourceRouletteSubsystem::Get(GetWorld()))
	{
		const TArray<FResourceNodeData>& ProcessedNodes = ResourceRouletteSubsystem->GetSessionRandomizedResourceNodes();
		UResourceRouletteUtility::RemoveExtractors(GetWorld(), ProcessedNodes, ResourceNodeSpawner->GetSpawnedResourceNodes());
	}
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
