#include "ResourceNodeMeshManager.h"
#include "EngineUtils.h"
#include "ProceduralGenerator.h"
#include "WorldSeedManager.h"
#include "ResourceNodeAssets.h"
#include "ResourceNodeUtility.h"

namespace ResourceNodeMeshManager
{
    UStaticMeshComponent* SetupNodeMeshComponent(UWorld* World, AFGResourceNode* CustomNode, const EOreType OreType, const FVector& Location, const TMap<EOreType, FResourceNodeAssets>& UniqueResourceNodeTypes)
    {
        AWorldSeedManager* SeedManager = nullptr;
        for (TActorIterator<AWorldSeedManager> It(World); It; ++It)
        {
            SeedManager = *It;
            break;
        }

        if (!SeedManager)
        {
            FResourceNodeUtilityLog::Get().LogMessage("Error: No AWorldSeedManager found in world for SetupNodeMeshComponent..",ELogLevel::Error);
            return nullptr;
        }

        AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(Location));
        if (!NewActor)
        {
            return nullptr;
        }

        UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(NewActor);
        if (!MeshComponent)
        {
            return nullptr;
        }

        if (const FResourceNodeAssets* ResourceNodeInfo = UniqueResourceNodeTypes.Find(OreType))
        {
            MeshComponent->SetStaticMesh(ResourceNodeInfo->GetMesh());
            MeshComponent->SetMaterial(0, ResourceNodeInfo->GetMaterial());
            MeshComponent->SetWorldScale3D(ResourceNodeInfo->GetScale());
        }

        SetMeshCollisionProfile(MeshComponent);
        MeshComponent->ComponentTags.Add(CustomResourceNodeTag);
        MeshComponent->SetMobility(EComponentMobility::Movable);
        NewActor->SetRootComponent(MeshComponent);
        MeshComponent->RegisterComponent();

        FProceduralGenerator Randomizer(SeedManager->GetGlobalSeed());
        FRotator Rotation = FRotator(0.0f, Randomizer.ProceduralRangeFloatByVector(Location, 0.0f, 360.0f), 0.0f);
        MeshComponent->SetWorldRotation(Rotation);

        return MeshComponent;
    }

    void AdjustMeshLocation(UStaticMeshComponent* MeshComponent, const FVector& BaseLocation, const FVector& Offset)
    {
        FVector Min, Max;
        MeshComponent->GetLocalBounds(Min, Max);
        FVector MeshPivot = (Min + Max) / 2.0f;

        FTransform MeshTransform = MeshComponent->GetComponentTransform();
        FVector AdjustedPivot = MeshTransform.TransformPosition(MeshPivot);

        FVector CorrectedLocation = BaseLocation - (AdjustedPivot - MeshTransform.GetLocation()) + Offset;
        MeshComponent->SetWorldLocation(CorrectedLocation);
    }

    void DestroyResourceNodeMeshes(const UWorld* World)
    {
        for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
        {
            UStaticMeshComponent* StaticMeshComponent = *It;
            if (StaticMeshComponent && StaticMeshComponent->GetWorld() == World)
            {
                if (UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh())
                {
                    if (FResourceNodeAssets::IsValidOreMeshPath(StaticMesh->GetPathName()) &&
                        !StaticMeshComponent->ComponentTags.Contains(CustomResourceNodeTag))
                    {
                        StaticMeshComponent->SetActive(false);
                        StaticMeshComponent->SetVisibility(false);
                        StaticMeshComponent->DestroyComponent();
                    }
                }
            }
        }
    }

    void SetMeshCollisionProfile(UStaticMeshComponent* MeshComponent)
    {
        MeshComponent->SetCollisionProfileName("ResourceMesh");
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
        MeshComponent->SetGenerateOverlapEvents(false);

        MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECollisionResponse::ECR_Block);
    }
}
