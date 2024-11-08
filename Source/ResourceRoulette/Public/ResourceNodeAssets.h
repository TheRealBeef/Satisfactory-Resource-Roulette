#pragma once

#include "CoreMinimal.h"
#include "FGResourceDescriptor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ResourceNodeUtility.h"

class FResourceNodeAssets
{
public:
    FResourceNodeAssets(EOreType InOreType, const FString& InMeshPath, const FString& InMaterialPath, const FVector& InOffset, const FVector& InScale);

    static FString GetOreTypeName(EOreType OreType);
    static bool IsValidOreMeshPath(const FString& MeshPath);
    static bool TryGetOreTypeByMeshPath(const FString& MeshPath, EOreType& OreType);
    static bool IsValidOreType(EOreType OreType);
    
    UStaticMesh* GetMesh() const;
    UMaterialInterface* GetMaterial() const;
    FVector GetOffset() const { return Offset; }
    FVector GetScale() const { return Scale; }
    EOreType GetOreType() const { return OreType; }
    const FString& GetMeshPath() const { return MeshPath; }
    const FString& GetMaterialPath() const { return MaterialPath; }
    TSubclassOf<UFGResourceDescriptor> GetResourceClass() const { return ResourceClass; }
    static FResourceNodeConfig GetResourceNodeConfigForOreType(EOreType OreType);
    
    void SetMeshPath(const FString& InMeshPath) { MeshPath = InMeshPath; Loaded = false; }
    void SetMaterialPath(const FString& InMaterialPath) { MaterialPath = InMaterialPath; Loaded = false; }
    void SetResourceClass(TSubclassOf<UFGResourceDescriptor> const InResourceClass) { ResourceClass = InResourceClass; }
    static const TArray<FResourceNodeAssets>& GetValidResourceNodeAssets();

private:
    mutable bool Loaded = false;
    EOreType OreType;
    FString MeshPath;
    FString MaterialPath;
    FVector Offset;
    FVector Scale;
    static TArray<FResourceNodeAssets> ValidResourceNodeAssets;
    TSubclassOf<UFGResourceDescriptor> ResourceClass = nullptr;
    mutable UStaticMesh* Mesh = nullptr;
    mutable UMaterialInterface* Material = nullptr;
    
    void LoadAssets() const;
};