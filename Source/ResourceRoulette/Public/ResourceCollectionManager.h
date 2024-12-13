#pragma once

#include "CoreMinimal.h"
#include "Resources/FGResourceDescriptor.h"
#include "Resources/FGResourceNodeBase.h"
#include "Resources/FGResourceNode.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.generated.h"

// // Information about the Node's mesh
// USTRUCT()
// struct FResourceNodeMeshData
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY()	UStaticMesh* Mesh = nullptr;
// 	UPROPERTY()	TArray<UMaterialInterface*> Materials;
// 	UPROPERTY()	FRotator Rotation = FRotator::ZeroRotator;
// 	UPROPERTY()	FVector Scale = FVector::OneVector;
// 	UPROPERTY()	FVector Location = FVector::ZeroVector;
// 	UPROPERTY()	FString MeshPath;
// 	UPROPERTY()	TArray<FString> MaterialPaths;
// };
//
// USTRUCT()
// struct FResourceNodeDecalData
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY()	UMaterial* DecalMaterial = nullptr;
// 	UPROPERTY()	FRotator Rotation = FRotator::ZeroRotator;
// 	UPROPERTY()	FVector Scale = FVector(1.0f);
// 	UPROPERTY()	FVector Location = FVector::ZeroVector;
// 	UPROPERTY()	float DecalSize = 1.0f;
// 	UPROPERTY()	FString DecalMaterialPath;
// };
//
// USTRUCT()
// struct FResourceNodeVisualData
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY()	bool bIsUsingDecal = false;
// 	UPROPERTY()	FResourceNodeMeshData MeshData;
// 	UPROPERTY()	FResourceNodeDecalData DecalData;
// };

// Information about the Node (includes the Mesh Data inside if uncommented)
USTRUCT()
struct FResourceNodeData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)	FString Classname;
	UPROPERTY(SaveGame)	FVector Location = FVector::ZeroVector;
	UPROPERTY(SaveGame)	FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(SaveGame)	FVector Offset = FVector::ZeroVector;
	UPROPERTY(SaveGame)	FVector Scale = FVector::OneVector;
	UPROPERTY(SaveGame)	TEnumAsByte<EResourcePurity> Purity;
	UPROPERTY(SaveGame)	TEnumAsByte<EResourceAmount> Amount;
	UPROPERTY(SaveGame)	EResourceNodeType ResourceNodeType;
	UPROPERTY(SaveGame)	FName ResourceClass;
	UPROPERTY(SaveGame)	EResourceForm ResourceForm;
	UPROPERTY(SaveGame)	bool bCanPlaceResourceExtractor = false;
	UPROPERTY(SaveGame)	bool bIsOccupied = false;
	UPROPERTY(SaveGame)	bool IsRayCasted = false;
	UPROPERTY(SaveGame)	FGuid NodeGUID;
	// UPROPERTY() FResourceNodeVisualData VisualData;
};


UCLASS()
class RESOURCEROULETTE_API UResourceCollectionManager : public UObject
{
	GENERATED_BODY()

public:
	UResourceCollectionManager();
	TArray<FResourceNodeData>& GetCollectedResourceNodes() { return CollectedResourceNodes; }
	void SetCollectedResourcesNodes(const TArray<FResourceNodeData>& InCollectedResourceNodes);
	void CollectWorldResources(const UWorld* World);
	// FResourceNodeVisualData CollectMeshData(AFGResourceNode* ResourceNode);
	void LogCollectedResources() const;
	// void LogCollectedResourcesMegaLog() const;


private:
	UPROPERTY()	TArray<FResourceNodeData> CollectedResourceNodes;
};
