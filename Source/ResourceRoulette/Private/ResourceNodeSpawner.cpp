#include "ResourceNodeSpawner.h"
#include "FGResourceNode.h"
#include "FGResourceNodeRepresentation.h"
#include "FGBuildableResourceExtractor.h"
#include "FGPortableMiner.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"

#include "ResourceNodePurityManager.h"
#include "ResourceNodeMeshManager.h"
#include "ResourceNodeUtility.h"
#include "ResourceNodeAssets.h"
#include "ProceduralGenerator.h"
#include "ResourceNodeGrouper.h"

UResourceNodeSpawner::UResourceNodeSpawner()
{
    PurityManager = CreateDefaultSubobject<UResourceNodePurityManager>(TEXT("PurityManager"));
}

int UResourceNodeSpawner::GetScannedUniqueResourceTypeSize() const
{
    return UniqueResourceNodeTypes.Num();
}

bool UResourceNodeSpawner::IsAllResourcesAlreadyScanned() const
{
    return UniqueResourceNodeTypes.Num() >= MaxUniqueClasses;
}

const TMap<EOreType, FResourceNodeAssets>& UResourceNodeSpawner::GetUniqueResourceNodeTypes()
{
    return UniqueResourceNodeTypes;
}

bool UResourceNodeSpawner::WorldIsLoaded(const UWorld* World)
{
    for (TActorIterator<AFGResourceNode> It(World); It; ++It)
    {
        return true;
    }
    return false;
}

void UResourceNodeSpawner::SetSeed(const int32 Seed)
{
    this->SpawnerSeed = Seed;
}
/////////////////////////////////////////////////////////////////////////
// Collects all the classes of nodes in the world
// TODO - it's only collecting 8 classes not 10, so something is wrong
// TODO - Remove the extraneous log data once we have solved the above
/////////////////////////////////////////////////////////////////////////
void UResourceNodeSpawner::CollectUniqueResourceClassesAndTypes(UWorld* World, TSet<TSubclassOf<UFGResourceDescriptor>>& UniqueResourceClasses)
{
    for (TActorIterator<AFGResourceNode> It(World); It; ++It)
    {
        if (UniqueResourceClasses.Num() >= MaxUniqueClasses)
        {
            break;
        }
        const AFGResourceNode* ResourceNode = *It;

        if (!ResourceNode)
        {
            FResourceNodeUtilityLog::Get().LogMessage("Found a null ResourceNode pointer.", ELogLevel::Warning);
            continue;
        }

        TSubclassOf<UFGResourceDescriptor> ResourceClass = ResourceNode->GetResourceClass();
        if (!ResourceClass)
        {
            FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
                TEXT("Resource Node %s has no ResourceClass."), *ResourceNode->GetName()), ELogLevel::Warning);
            continue;
        }

        FString ResourceClassName = ResourceClass->GetName();

        FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
            TEXT("Found Resource Node Class: %s"), *ResourceClassName), ELogLevel::Debug);

        if (ResourceNode->GetResourceAmount() != EResourceAmount::RA_Infinite)
        {
            UniqueResourceClasses.Add(ResourceClass);
            FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
                TEXT("Collected unique resource class: %s"), *ResourceClassName), ELogLevel::Debug);
        }
    }
}

/////////////////////////////////////////////////////////////////////////
// One of the two "Core" methods, this one pulls all the
// Information about the nodes in the world
/////////////////////////////////////////////////////////////////////////
void UResourceNodeSpawner::ScanWorldResourceNodes(UWorld* World)
{
    if (!World)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Skipping ScanWorldResourceNodes: World is invalid", ELogLevel::Error);
        return;
    }
    if (IsAllResourcesAlreadyScanned())
    {
        FResourceNodeUtilityLog::Get().LogMessage("Skipping ScanWorldResourceNodes: Already scanned", ELogLevel::Debug);
        return;
    }

    // Counts all the node purities separately so we can match it when we spawn the new ones
    PurityManager->CountNodePurities(World);
    const TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts = PurityManager->GetOriginalPurityCounts();
    
    TSet<TSubclassOf<UFGResourceDescriptor>> UniqueResourceClasses;
    CollectUniqueResourceClassesAndTypes(World, UniqueResourceClasses);

    FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
    TEXT("UniqueResourceClasses size after collection: %d (MaxUniqueClasses: %d)"), 
        UniqueResourceClasses.Num(), MaxUniqueClasses), ELogLevel::Debug);
    
    if (UniqueResourceClasses.Num() < MaxUniqueClasses)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Not enough unique classes collected. Exiting scan.", ELogLevel::Debug);
        return;
    }

    InitializeUniqueResourceNodeTypes(UniqueResourceClasses);
}

void UResourceNodeSpawner::InitializeUniqueResourceNodeTypes(const TSet<TSubclassOf<UFGResourceDescriptor>>& UniqueResourceClasses)
{
    for (const auto& ResourceClass : UniqueResourceClasses)
    {
        UTexture2D* CompasTexture = UFGResourceDescriptor::GetCompasTexture(ResourceClass);
        if (!CompasTexture) continue;
        FString TexturePath = CompasTexture->GetPathName();
        EOreType CurrentOreType = EOreType::Bauxite;
        bool bFound = false;
        for (const auto& OrePair : OreTypeNameList)
        {
            if (TexturePath.Contains(OrePair.Key, ESearchCase::IgnoreCase))
            {
                CurrentOreType = OrePair.Value;
                bFound = true;
                break;
            }
        }

        if (bFound)
        {
            AddResourceNodeType(CurrentOreType, ResourceClass);
            FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
                TEXT("Added resource node type: %s with class: %s"),
                *UEnum::GetValueAsString(CurrentOreType), *ResourceClass->GetName()), ELogLevel::Debug);
        }
    }
}

void UResourceNodeSpawner::AddResourceNodeType(const EOreType OreType, const TSubclassOf<UFGResourceDescriptor> ResourceClass)
{
    FResourceNodeConfig Config = FResourceNodeAssets::GetResourceNodeConfigForOreType(OreType);
    FResourceNodeAssets Asset(OreType, Config.MeshPath, Config.MaterialPath, Config.Offset, Config.Scale);
    Asset.SetResourceClass(ResourceClass);
    UniqueResourceNodeTypes.Add(OreType, Asset);
    FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
        TEXT("UniqueResourceNodeTypes now has %d entries after adding %s"), 
        UniqueResourceNodeTypes.Num(), *UEnum::GetValueAsString(OreType)), ELogLevel::Debug);
}

/////////////////////////////////////////////////////////////////////////
// This is the 2nd "Core" method - this one replaces all the resource nodes
// With new nodes that are pseudorandom based on the seed
/////////////////////////////////////////////////////////////////////////

void UResourceNodeSpawner::ReplaceStandardResourceNodesUpdate(UWorld* World)
{
    if (!IsAllResourcesAlreadyScanned())
    {
        FResourceNodeUtilityLog::Get().LogMessage("Not all resources are scanned. Exiting ReplaceStandardResourceNodesUpdate.", ELogLevel::Warning);
        return;
    }
    FResourceNodeUtilityLog::Get().LogMessage("Starting ReplaceStandardResourceNodesUpdate...", ELogLevel::Debug);
    TMap<EOreType, TMap<EResourcePurity, int>> RemainingPurityCounts = PurityManager->GetOriginalPurityCounts();
    FProceduralGenerator Randomizer(this->SpawnerSeed);
    ResourceNodeMeshManager::DestroyResourceNodeMeshes(World);
    TArray<FCustomResourceNode> StandardResourceNodes = CollectStandardResourceNodes(World);
    ProcessResourceNodeGroups(World, StandardResourceNodes, RemainingPurityCounts, Randomizer);
    ShuffleAndReplaceNodes(World, RemainingPurityCounts, Randomizer);

    AssignMinersToNodes(World);
    AssignPortableMinersToNodes(World);
}

TArray<FCustomResourceNode> UResourceNodeSpawner::CollectStandardResourceNodes(UWorld* World)
{
    TArray<FCustomResourceNode> StandardResourceNodes;
    for (TActorIterator<AFGResourceNode> It(World); It; ++It)
    {
        AFGResourceNode* ResourceNode = *It;
        if (IsValidResourceNode(ResourceNode))
        {
            EOreType OreType;
            if (UResourceNodeUtility::GetOreTypeFromResourceNode(ResourceNode, OreType))
            {
                StandardResourceNodes.Emplace(ResourceNode->GetActorLocation(), OreType, ResourceNode->GetResoucePurity());
                ResourceNode->Destroy();
            }
        }
    }
    return StandardResourceNodes;
}

bool UResourceNodeSpawner::IsValidResourceNode(AFGResourceNode* ResourceNode) const
{
    return ResourceNode &&
           ResourceNode->GetResourceNodeType() == EResourceNodeType::Node &&
           ResourceNode->GetResourceForm() == EResourceForm::RF_SOLID &&
           ResourceNode->GetResourceAmount() == EResourceAmount::RA_Infinite &&
           !CustomResourceNodeMap.Contains(ResourceNode);
}


FVector UResourceNodeSpawner::GetCenterOfVectors(const TArray<FVector>& VectorList)
{
    int32 Count = VectorList.Num();
    if (Count == 0) return FVector(); 

    FVector VectorSum(0.0f, 0.0f, 0.0f);
    for (const FVector& Vector : VectorList)
    {
        VectorSum += Vector;
    }

    return VectorSum / Count;
}

TTuple<AFGResourceNode*, float> UResourceNodeSpawner::GetClosestCustomResourceNode(UWorld* World, const FVector& Location, const EOccupiedType OccupiedStatus)
{
    TTuple<AFGResourceNode*, float> ClosestNode(nullptr, FLT_MAX);

    for (const auto& CustomResourceNodeEntry : CustomResourceNodeMap)
    {
        AFGResourceNode* ResourceNode = CustomResourceNodeEntry.Key.Get();
        if (!ResourceNode) continue;
        if (OccupiedStatus != EOccupiedType::Any)
        {
            bool IsOccupied = ResourceNode->IsOccupied();
            if ((OccupiedStatus == EOccupiedType::Unoccupied && IsOccupied) ||
                (OccupiedStatus == EOccupiedType::Occupied && !IsOccupied))
            {
                continue;
            }
        }
        float CurrentDistance = FVector::Dist(Location, ResourceNode->GetActorLocation());
        if (CurrentDistance < ClosestNode.Value)
        {
            ClosestNode = MakeTuple(ResourceNode, CurrentDistance);
        }
    }

    return ClosestNode;
}

void UResourceNodeSpawner::ProcessResourceNodeGroups(UWorld* World, TArray<FCustomResourceNode>& StandardResourceNodes, TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts, FProceduralGenerator& Randomizer)
{
    for (auto& ResourceNode : StandardResourceNodes)
    {
        NodeGrouper.AddResourceNode(ResourceNode);
    }
    
    // TODO - this is a hardcoded value - maybe we should define it in Utility file so we centralize these kinda of things
    NodeGrouper.OrganizeResourceNodeGroups(7000.0f); // 70m radius

    for (auto& Group : NodeGrouper.GetResourceNodeGroups())
    {
        TArray<FVector> GroupLocations;
        for (const auto& ResourceNode : Group)
        {
            GroupLocations.Add(ResourceNode.GetLocation());
        }

        FVector Center = GetCenterOfVectors(GroupLocations);
        int RandomResourceTypeIndex = Randomizer.ProceduralRangeIntByVector(Center, 0, MakeGroupTypes.Num() - 1);
        EOreType NewResourceOreType = MakeGroupTypes[RandomResourceTypeIndex];

        for (const auto& ResourceNode : Group)
        {
            EResourcePurity PurityToAssign = UResourceNodePurityManager::DeterminePurityToAssign(NewResourceOreType, RemainingPurityCounts);
            SpawnCustomResourceNode(World, NewResourceOreType, ResourceNode.GetLocation(), PurityToAssign, true);
        }
    }
}

void UResourceNodeSpawner::ShuffleAndReplaceNodes(UWorld* World, TMap<EOreType, TMap<EResourcePurity, int>>& RemainingPurityCounts, FProceduralGenerator& Randomizer)
{
    TArray<FCustomResourceNode> CustomResourceNodesList;
    for (auto& OreArray : NodeGrouper.GetStandardResourceNodeList())
    {
        if (ShuffleTypes.Contains(OreArray.Key))
        {
            CustomResourceNodesList.Append(OreArray.Value);
        }
    }

    const int32 LastIndex = CustomResourceNodesList.Num() - 1;
    for (int32 i = LastIndex; i > 0; i--)
    {
        FVector LocationForSwap = CustomResourceNodesList[i].GetLocation();
        int32 SwapIndex = Randomizer.ProceduralRangeIntByVector(LocationForSwap, 0, i);

        if (i != SwapIndex)
        {
            Swap(CustomResourceNodesList[i], CustomResourceNodesList[SwapIndex]);
        }
    }

    for (auto& ResourceNode : CustomResourceNodesList)
    {
        EOreType OreType = ResourceNode.GetOreType();
        EResourcePurity PurityToAssign = UResourceNodePurityManager::DeterminePurityToAssign(OreType, RemainingPurityCounts);
        SpawnCustomResourceNode(World, OreType, ResourceNode.GetLocation(), PurityToAssign, true);
    }
}

void UResourceNodeSpawner::AssignMinersToNodes(UWorld* World)
{
    for (TActorIterator<AFGBuildableResourceExtractor> It(World); It; ++It)
    {
        AFGBuildableResourceExtractor* ResourceExtractor = *It;
        if (CustomResourceNodeMap.Contains(Cast<AFGResourceNode>(ResourceExtractor->mExtractableResource)) ||
            ResourceExtractor->GetExtractorTypeName() != TEXT("Miner"))
        {
            continue;
        }
        FVector ExtractorLocation = ResourceExtractor->GetActorLocation() - FVector(0.0f, 0.0f, 150.0f);
        TPair<AFGResourceNode*, float> ClosestNode = GetClosestCustomResourceNode(World, ExtractorLocation, EOccupiedType::Unoccupied);

        // TODO - this is a hardcoded value - maybe we should define it in Utility file so we centralize these kinda of things
        if (ClosestNode.Key && ClosestNode.Value < 700.0f) // 7m range for  miners
        {
            ClosestNode.Key->SetIsOccupied(true);
            ResourceExtractor->mExtractableResource = ClosestNode.Key;
        }
    }
}

void UResourceNodeSpawner::AssignPortableMinersToNodes(UWorld* World)
{
    for (TActorIterator<AFGPortableMiner> It(World); It; ++It)
    {
        AFGPortableMiner* PortableMiner = *It;
        if (CustomResourceNodeMap.Contains(Cast<AFGResourceNode>(PortableMiner->mExtractResourceNode)))
        {
            continue;
        }

        FVector ExtractorLocation = PortableMiner->GetActorLocation();
        TPair<AFGResourceNode*, float> ClosestNode = GetClosestCustomResourceNode(World, ExtractorLocation, EOccupiedType::Any);

        // TODO - this is a hardcoded value - maybe we should define it in Utility file so we centralize these kinda of things
        if (ClosestNode.Key && ClosestNode.Value < 1500.0f) // 15m range for portable miners
        {
            PortableMiner->mExtractResourceNode = ClosestNode.Key;
        }
    }
}

bool UResourceNodeSpawner::SpawnCustomResourceNode(UWorld* World, const EOreType OreType, const FVector& Location, const EResourcePurity Purity, bool bUseRaycastAdjust)
{
    if (!IsAllResourcesAlreadyScanned())
    {
        FResourceNodeUtilityLog::Get().LogMessage("Resources are not fully scanned; aborting SpawnCustomResourceNode.",ELogLevel::Warning);
        return false;
    }

    FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(TEXT("Spawning CustomNode with OreType: %s, Purity: %d at Location: %s"),
        *FResourceNodeAssets::GetOreTypeName(OreType), static_cast<int>(Purity), *Location.ToString()),ELogLevel::Debug);

    AFGResourceNode* CustomNode = World->SpawnActor<AFGResourceNode>(ResourceNodeClass, Location, FRotator());
    if (!CustomNode)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Error: Failed to spawn AFGResourceNode in SpawnCustomResourceNode.",ELogLevel::Error);
        return false;
    }

    InitializeResourceNodeComponents(CustomNode, OreType, Purity, Location);

    UStaticMeshComponent* MeshComponent = ResourceNodeMeshManager::SetupNodeMeshComponent(World, CustomNode, OreType, Location, UniqueResourceNodeTypes);
    if (!MeshComponent)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Error: Failed to setup mesh component for CustomNode.",ELogLevel::Error);
        return false;
    }

    ResourceNodeMeshManager::AdjustMeshLocation(MeshComponent, Location, UniqueResourceNodeTypes[OreType].GetOffset());

    CustomResourceNodeMap.Add(CustomNode, MeshComponent);
    FResourceNodeUtilityLog::Get().LogMessage("Custom resource node successfully spawned and initialized.",ELogLevel::Debug);

    return true;
}

void UResourceNodeSpawner::InitializeResourceNodeComponents(AFGResourceNode* CustomNode, const EOreType OreType, const EResourcePurity Purity, const FVector& Location)
{
    if (!CustomNode)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Error: CustomNode is null in InitializeResourceNodeComponents.",ELogLevel::Warning);
        return;
    }
    const FResourceNodeAssets* ResourceNodeAsset = nullptr;
    for (const auto& Asset : FResourceNodeAssets::GetValidResourceNodeAssets())
    {
        if (Asset.GetOreType() == OreType)
        {
            ResourceNodeAsset = &Asset;
            break;
        }
    }

    if (!ResourceNodeAsset)
    {
        FResourceNodeUtilityLog::Get().LogMessage("Error: ResourceNodeAsset not found for OreType in InitializeResourceNodeComponents.",ELogLevel::Error);
        return;
    }
    CustomNode->InitResource(ResourceNodeAsset->GetResourceClass(), EResourceAmount::RA_Infinite, Purity);
    CustomNode->mResourceNodeType = EResourceNodeType::Node;
    CustomNode->mCanPlacePortableMiner = true;
    CustomNode->mCanPlaceResourceExtractor = true;
    if (!CustomNode->GetRootComponent())
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(CustomNode);
        MeshComp->RegisterComponent();
        CustomNode->SetRootComponent(MeshComp);
    }
    CustomNode->GetRootComponent()->SetMobility(EComponentMobility::Movable);
    CustomNode->SetActorLocation(Location);
    if (!CustomNode->mResourceNodeRepresentation)
    {
        CustomNode->mResourceNodeRepresentation = NewObject<UFGResourceNodeRepresentation>(CustomNode);
    }
    CustomNode->mResourceNodeRepresentation->SetupResourceNodeRepresentation(CustomNode);
    CustomNode->UpdateMeshFromDescriptor();
    CustomNode->UpdateNodeRepresentation();
    CustomNode->InitRadioactivity();
    CustomNode->UpdateRadioactivity();
    if (CustomNode->mBoxComponent)
    {
        CustomNode->mBoxComponent->SetWorldLocation(Location);
        CustomNode->mBoxComponent->SetWorldScale3D(FVector(30.0f, 30.0f, 2.0f));
        CustomNode->mBoxComponent->SetCollisionProfileName("Resource");
    }
}

void UResourceNodeSpawner::ResetResources()
{
    ResourceNodeClass = nullptr;
    CustomResourceNodeMap.Empty();
    UniqueResourceNodeTypes.Empty();
}