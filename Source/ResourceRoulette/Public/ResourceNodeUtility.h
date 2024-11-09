#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "FGResourceNode.h"
#include "FGBuildableResourceExtractor.h"
#include "GameInstanceModule.h"
#include "Misc/OutputDeviceFile.h"
#include "ResourceNodeUtility.generated.h"

// DECLARE_LOG_CATEGORY_EXTERN(LogResourceRoulette, Warning, All);
constexpr int MaxUniqueClasses = 10; // number of ore types in EOreType

struct FResourceNodeConfig
{
	FString MeshPath;
	FString MaterialPath;
	FVector Offset;
	FVector Scale;
};

UENUM(BlueprintType)
enum class EOccupiedType : uint8
{
	Any,
	Occupied,
	Unoccupied,
};

UENUM(BlueprintType)
enum class EOreType : uint8
{
	Bauxite,
	Caterium,
	Coal,
	Copper,
	Iron,
	Limestone,
	Quartz,
	Sam,
	Sulfur,
	Uranium,
};

// Mapping from ore type names to their enum values
const TArray<TPair<FString, EOreType>> OreTypeNameList = {
	MakeTuple("Bauxite", EOreType::Bauxite),
	MakeTuple("Caterium", EOreType::Caterium),
	MakeTuple("Coal", EOreType::Coal),
	MakeTuple("Copper", EOreType::Copper),
	MakeTuple("Iron", EOreType::Iron),
	MakeTuple("Stone", EOreType::Limestone),
	MakeTuple("Quartz", EOreType::Quartz),
	MakeTuple("Sam", EOreType::Sam),
	MakeTuple("Sulfur", EOreType::Sulfur),
	MakeTuple("Uranium", EOreType::Uranium)
};

// Ore types used for shuffling in the procedural generation process
const TArray<EOreType> ShuffleTypes = {
	EOreType::Bauxite,
	EOreType::Caterium,
	EOreType::Coal,
	EOreType::Copper,
	EOreType::Iron,
	EOreType::Limestone,
	EOreType::Quartz,
	EOreType::Sulfur
};

// Ore types used for grouping resources
const TArray<EOreType> MakeGroupTypes = {
	EOreType::Caterium,
	EOreType::Coal,
	EOreType::Copper,
	EOreType::Iron,
	EOreType::Limestone,
	EOreType::Quartz,
	EOreType::Sulfur
};

enum class ELogLevel
{
	Debug = 2,
	Warning = 1,
	Error = 0
};

UENUM(BlueprintType)
enum class EBlueprintLogLevel : uint8
{
	Error UMETA(DisplayName = "Error"),
	Warning UMETA(DisplayName = "Warning"),
	Debug UMETA(DisplayName = "Debug")
};


const FName CustomResourceNodeTag = "CustomResourceNode";

class FResourceNodeUtilityLog
{
public:
	static FResourceNodeUtilityLog& Get();

	void InitializeLog();
	void ShutdownLog();
	void LogMessage(const FString& Message, ELogLevel Level);
	void SetUseCustomLogFile(bool bEnableCustomLogFile);
	
private:
	bool bUseCustomLogFile = true;
	
	FResourceNodeUtilityLog() = default;
	FResourceNodeUtilityLog(const FResourceNodeUtilityLog&) = delete;
	FResourceNodeUtilityLog& operator=(const FResourceNodeUtilityLog&) = delete;

	void RawLogMessage(const FString& Message);
	
	FString* LogFile = nullptr;
	FCriticalSection LogFileMutex;

	static FResourceNodeUtilityLog Instance;
};

UCLASS()
class RESOURCEROULETTE_API UResourceNodeUtility: public UObject
{

GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Resource Roulette")
	static void UseCustomLogFile(bool bEnableCustomLogFile);

	UFUNCTION(BlueprintCallable, Category = "Resource Roulette")
	static void LogMessage(const FString& Message, EBlueprintLogLevel Level);
	
	UFUNCTION(BlueprintCallable, Category = "Resource Roulette")
	static void InitializeLoggingModule();
	
	static bool TryGetOreTypeFromName(const FString& Name, EOreType& OreType);
	static bool TryGetNameFromOreType(EOreType OreType, FString& Name);
	static bool GetOreTypeFromResourceNode(const AFGResourceNode* ResourceNode, EOreType& OutOreType);
	static bool FindOreTypeFromPath(const FString& Path, EOreType& OutOreType);
	static void LogAllResourceNodes(const UWorld* World);
};