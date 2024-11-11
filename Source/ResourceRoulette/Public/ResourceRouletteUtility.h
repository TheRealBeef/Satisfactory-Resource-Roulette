#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Resources/FGResourceNode.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Misc/OutputDeviceFile.h"
#include "ResourceRouletteUtility.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSatisfactoryModLoader, Log, All);

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

class FResourceRouletteUtilityLog
{
public:
	static FResourceRouletteUtilityLog& Get();

	void InitializeLog();
	void ShutdownLog();
	void LogMessage(const FString& Message, ELogLevel Level);
	void SetUseCustomLogFile(bool bEnableCustomLogFile);

private:
	bool bUseCustomLogFile = true;

	FResourceRouletteUtilityLog() = default;
	FResourceRouletteUtilityLog(const FResourceRouletteUtilityLog&) = delete;
	FResourceRouletteUtilityLog& operator=(const FResourceRouletteUtilityLog&) = delete;

	void RawLogMessage(const FString& Message);

	FString* LogFile = nullptr;
	FCriticalSection LogFileMutex;

	static FResourceRouletteUtilityLog Instance;
};

UCLASS()
class RESOURCEROULETTE_API UResourceRouletteUtility : public UObject
{
	GENERATED_BODY()

public:
	static const TArray<FName> ValidResourceClasses;

	UFUNCTION(BlueprintCallable, Category = "Resource Roulette")
	static void UseCustomLogFile(bool bEnableCustomLogFile);

	UFUNCTION(BlueprintCallable, Category = "Resource Roulette")
	static void LogMessage(const FString& Message, EBlueprintLogLevel Level);

	UFUNCTION(BlueprintCallable, Category = "Resource Roulette")
	static void InitializeLoggingModule();

	static bool IsValidResourceClass(const FName& ResourceClassName);
	static bool IsValidInfiniteResourceNode(const AFGResourceNode* ResourceNode);
	static void LogAllResourceNodes(const UWorld* World);
};
