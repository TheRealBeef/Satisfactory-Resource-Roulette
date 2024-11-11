#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

USTRUCT()
struct FResourceRouletteAssetSolid
{
    GENERATED_BODY()

    UPROPERTY() FString MeshPath;
    UPROPERTY() TArray<FString> MaterialPaths;
    UPROPERTY() FVector MeshOffset;
    UPROPERTY() FVector MeshScale;
};
USTRUCT()

struct FResourceRouletteAssetHeat
{
    GENERATED_BODY()

    UPROPERTY() FString MeshPath;
    UPROPERTY() TArray<FString> MaterialPaths;
    UPROPERTY() FVector MeshOffset;
    UPROPERTY() FVector MeshScale;
};

USTRUCT()
struct FResourceRouletteAssetLiquid
{
    GENERATED_BODY()
    
    UPROPERTY() TArray<FString> MaterialPaths;
    UPROPERTY() float DecalSize;;

};

USTRUCT()
struct FResourceRouletteAssetFracking
{
    GENERATED_BODY()

    UPROPERTY() TArray<FString> MeshPaths;
    UPROPERTY() TArray<FString> MaterialPaths;
    UPROPERTY() FVector MeshOffset;
    UPROPERTY() FVector MeshScale;
};


class UResourceRouletteAssets
{
public:

    static const TMap<FName, FResourceRouletteAssetSolid> SolidResourceInfoMap;
    static const TMap<FName, FResourceRouletteAssetHeat> HeatResourceInfoMap;
    static const TMap<FName, FResourceRouletteAssetLiquid> LiquidResourceInfoMap;
    static const TMap<FName, FResourceRouletteAssetFracking> FrackingResourceInfoMap;


    static FString GetSolidMesh (const FName& ResourceClass);
    static TArray<FString> GetSolidMaterial (const FName& ResourceClass);
    static FVector GetSolidOffset (const FName& ResourceClass);
    static FVector GetSolidScale (const FName& ResourceClass);


    static FString GetHeatMesh (const FName& ResourceClass);
    static TArray<FString> GetHeatMaterials (const FName& ResourceClass);
    static FVector GetHeatOffset (const FName& ResourceClass);
    static FVector GetHeatScale (const FName& ResourceClass);
    
    static TArray<FString> GetLiquidMaterials (const FName& ResourceClass);
    static float GetLiquidDecalScale (const FName& ResourceClass);
    
    static TArray<FString> GetFrackingMeshes(const FName& ResourceClass);
    static TArray<FString> GetFrackingMaterisl (const FName& ResourceClass);
    static FVector GetFrackingOffset (const FName& ResourceClass);
    static FVector GetFrackingScale (const FName& ResourceClass);
};
