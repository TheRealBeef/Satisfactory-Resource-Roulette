#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

#include "ResourceNodeAssets.h"
#include "ResourceNodeUtility.h"

namespace ResourceNodeMeshManager
{
	UStaticMeshComponent* SetupNodeMeshComponent(UWorld* World, AFGResourceNode* CustomNode, EOreType OreType, const FVector& Location, const TMap<EOreType, FResourceNodeAssets>& UniqueResourceNodeTypes);
	void AdjustMeshLocation(UStaticMeshComponent* MeshComponent, const FVector& BaseLocation, const FVector& Offset);
	void DestroyResourceNodeMeshes(const UWorld* World);
	void SetMeshCollisionProfile(UStaticMeshComponent* MeshComponent);
}
