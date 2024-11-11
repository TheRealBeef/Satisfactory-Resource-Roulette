#pragma once

#include "CoreMinimal.h"
#include "Resources/FGResourceDescriptor.h"
#include "Resources/FGResourceNodeBase.h"
#include "Resources/FGResourceNode.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ResourceRouletteUtility.h"
#include "ResourceCollectionManager.generated.h"

// Information about the Node's mesh
USTRUCT()
struct FResourceNodeMeshData
{
	GENERATED_BODY()
	
	UPROPERTY() UStaticMesh* Mesh = nullptr;
	UPROPERTY() TArray<UMaterialInterface*> Materials;
	UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY() FVector Scale = FVector::OneVector;
	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() FString MeshPath;
	UPROPERTY() TArray<FString> MaterialPaths;
};

USTRUCT()
struct FResourceNodeDecalData
{
	GENERATED_BODY()

	UPROPERTY() UMaterial* DecalMaterial = nullptr;
	UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY() FVector Scale = FVector(1.0f);
	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() float DecalSize = 1.0f;
	UPROPERTY() FString DecalMaterialPath;
};

USTRUCT()
struct FResourceNodeVisualData
{
	GENERATED_BODY()

	UPROPERTY() bool bIsUsingDecal = false;
	UPROPERTY() FResourceNodeMeshData MeshData;
	UPROPERTY() FResourceNodeDecalData DecalData;
};

// Information about the Node (includes the Mesh Data inside)
USTRUCT()
struct FResourceNodeData
{
	GENERATED_BODY()

	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY() FVector Scale = FVector::OneVector;
	UPROPERTY() TEnumAsByte<EResourcePurity> Purity;
	UPROPERTY() TEnumAsByte<EResourceAmount> Amount;
	UPROPERTY() TSubclassOf<UFGResourceDescriptor> ResourceClass;
	UPROPERTY() EResourceForm ResourceForm;
	UPROPERTY() bool bCanPlaceResourceExtractor = false;
	UPROPERTY() bool bIsOccupied = false;
	UPROPERTY() FResourceNodeVisualData VisualData;
};


UCLASS()
class RESOURCEROULETTE_API UResourceCollectionManager : public UObject
{
	GENERATED_BODY()

public:
	UResourceCollectionManager();

	void CollectWorldResources(const UWorld* World);
	void LogCollectedResources() const;
	void LogCollectedResourcesPeriodic();
	
private:
	
	FTimerHandle PeriodicLogTimerHandle;
	TArray<FResourceNodeData> CollectedResourceNodes;
};
