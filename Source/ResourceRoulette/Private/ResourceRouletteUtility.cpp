#include "ResourceRouletteUtility.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "Resources/FGResourceNode.h"
#include "HAL/IConsoleManager.h"
#include "ResourceCollectionManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY_STATIC(LogResourceRoulette, Log, All);

/// Allows log setting verbosity at runtine
static TAutoConsoleVariable<int32> CVarLogLevel(
	TEXT("ResourceRoulette.LogLevel"), 1,
	TEXT("Sets log verbosity level: 0 = Error, 1 = Warning, 2 = Debug"),
	ECVF_Default
);

FResourceRouletteUtilityLog FResourceRouletteUtilityLog::Instance;

FResourceRouletteUtilityLog& FResourceRouletteUtilityLog::Get()
{
	return Instance;
}

/// Initializes Logfile stuff
void FResourceRouletteUtilityLog::InitializeLog()
{
	if (bUseCustomLogFile)
	{
		FScopeLock Lock(&LogFileMutex);
		if (!LogFile)
		{
			const FString LogFilePath = FPaths::Combine(
				FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA")),
				TEXT("FactoryGame/Saved/Logs/ResourceRoulette.log")
			);

			IFileManager& FileManager = IFileManager::Get();
			FileManager.Delete(*LogFilePath);
			LogFile = new FString(LogFilePath);

			RawLogMessage(TEXT("Resource Roulette Module Log Startup"));
		}
	}
}

/// @param bEnableCustomLogFile Whether to use Custom Logfile or UE_Log
void FResourceRouletteUtilityLog::SetUseCustomLogFile(const bool bEnableCustomLogFile)
{
	bUseCustomLogFile = bEnableCustomLogFile;
}

/// Writes log message to file
/// @param Message 
void FResourceRouletteUtilityLog::RawLogMessage(const FString& Message)
{
	FScopeLock Lock(&LogFileMutex);
	if (LogFile && !LogFile->IsEmpty())
	{
		const FString LogFilePath = *LogFile;
		const FString FormattedMessage = Message + LINE_TERMINATOR;

		FFileHelper::SaveStringToFile(FormattedMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect,
		                              &IFileManager::Get(), FILEWRITE_Append);
	}
}

/// C++ Callable version of custom log
/// @param Message 
/// @param Level 
void FResourceRouletteUtilityLog::LogMessage(const FString& Message, ELogLevel Level)
{
	int32 CurrentLogLevel = CVarLogLevel.GetValueOnAnyThread();
	if (static_cast<int32>(Level) >= CurrentLogLevel)
	{
		if (bUseCustomLogFile && LogFile)
		{
			RawLogMessage(Message);
		}
		else
		{
			switch (Level)
			{
			case ELogLevel::Error:
				UE_LOG(LogResourceRoulette, Error, TEXT("%s"), *Message);
				break;
			case ELogLevel::Warning:
				UE_LOG(LogResourceRoulette, Warning, TEXT("%s"), *Message);
				break;
			case ELogLevel::Debug:
			default:
				UE_LOG(LogResourceRoulette, Log, TEXT("%s"), *Message);
				break;
			}
		}
	}
}


/// Blueprint callable way to override using custom logfile or not
/// @param bEnableCustomLogFile 
void UResourceRouletteUtility::UseCustomLogFile(const bool bEnableCustomLogFile)
{
	FResourceRouletteUtilityLog::Get().SetUseCustomLogFile(bEnableCustomLogFile);
}

/// Blueprint callable version of custom log
/// @param Message 
/// @param Level 
void UResourceRouletteUtility::LogMessage(const FString& Message, EBlueprintLogLevel Level)
{
	FResourceRouletteUtilityLog& Logger = FResourceRouletteUtilityLog::Get();
	Logger.LogMessage(Message, static_cast<ELogLevel>(Level));
}

/// Shuts down the log
void FResourceRouletteUtilityLog::ShutdownLog()
{
	if (bUseCustomLogFile && LogFile)
	{
		RawLogMessage(TEXT("Resource Roulette Module Log Shutdown"));
		delete LogFile;
		LogFile = nullptr;
	}
}

/// Static method to initialize the logging module
void UResourceRouletteUtility::InitializeLoggingModule()
{
	FResourceRouletteUtilityLog::Get().InitializeLog();
	FResourceRouletteUtilityLog::Get().LogMessage("Logging initialized", ELogLevel::Debug);
}

/// Checks to see if it's one of the resources we want to mess with
/// @param ResourceClassName 
/// @return 
bool UResourceRouletteUtility::IsValidResourceClass(const FName& ResourceClassName)
{
	return !ResourceClassName.IsNone() && ValidResourceClasses.Contains(ResourceClassName);
}

/// Checks to see if it's valid resource class and infinite, which should filter out deposits
/// @param ResourceNode 
/// @return 
bool UResourceRouletteUtility::IsValidInfiniteResourceNode(const AFGResourceNode* ResourceNode)
{
	if (!ResourceNode || ResourceNode->GetResourceAmount() != EResourceAmount::RA_Infinite)
	{
		return false;
	}

	const FName ResourceClassName = ResourceNode && ResourceNode->GetResourceClass()
		                                ? ResourceNode->GetResourceClass()->GetFName()
		                                : NAME_None;

	return IsValidResourceClass(ResourceClassName);
}

const TArray<FName> UResourceRouletteUtility::ValidResourceClasses = {
	// "Desc_NitrogenGas_C", // This is a fracking node, it's on TODO
	// "Desc_Geyser_C", // We'll come back to geysers later TODO
	// "Desc_LiquidOil_C", // Until I can separate fracking nodes, it's on TODO too :(
	// "Desc_Water_C", // This is a fracking node, it's on TODO
	"Desc_SAM_C",
	"Desc_Stone_C",
	"Desc_OreIron_C",
	"Desc_OreCopper_C",
	"Desc_OreGold_C",
	"Desc_Coal_C",
	"Desc_RawQuartz_C",
	"Desc_Sulfur_C",
	"Desc_OreBauxite_C",
	"Desc_OreUranium_C",
	"Desc_FF_Dirt_Fertilized_C",
	"Desc_FF_Dirt_C",
	"Desc_FF_Dirt_Wet_C",
	// "Desc_RP_Deanium_C", // This is a fracking node, it's on TODO
	// "Desc_RP_WaterDamNode_C", // We shouldn't randomize this
	// "Desc_WaterTurbineNode_C", // We shouldn't randomize this
	"Desc_RP_Thorium_C"
};

/// Logs all the resource nodes in the world
/// @param World World Context
void UResourceRouletteUtility::LogAllResourceNodes(const UWorld* World)
{
	FResourceRouletteUtilityLog::Get().LogMessage("=== Begin Logging All AFGResourceNodes in the World ===",
	                                              ELogLevel::Debug);

	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
	{
		const AFGResourceNode* ResourceNode = *It;

		FString NodeName = ResourceNode->GetName();
		FString ResourceClass = ResourceNode->GetResourceClass()
			                        ? ResourceNode->GetResourceClass()->GetName()
			                        : TEXT("None");

		FString ResourceForm;
		switch (ResourceNode->GetResourceForm())
		{
		case EResourceForm::RF_SOLID: ResourceForm = TEXT("Solid");
			break;
		case EResourceForm::RF_LIQUID: ResourceForm = TEXT("Liquid");
			break;
		case EResourceForm::RF_GAS: ResourceForm = TEXT("Gas");
			break;
		case EResourceForm::RF_HEAT: ResourceForm = TEXT("Heat");
			break;
		default: ResourceForm = FString::Printf(
				TEXT("Unknown (%d)"), static_cast<int32>(ResourceNode->GetResourceForm()));
			break;
		}

		FString ResourceAmount;
		switch (ResourceNode->GetResourceAmount())
		{
		case EResourceAmount::RA_Poor: ResourceAmount = TEXT("Poor");
			break;
		case EResourceAmount::RA_Normal: ResourceAmount = TEXT("Normal");
			break;
		case EResourceAmount::RA_Rich: ResourceAmount = TEXT("Rich");
			break;
		case EResourceAmount::RA_Infinite: ResourceAmount = TEXT("Infinite");
			break;
		default: ResourceAmount = FString::Printf(
				TEXT("Unknown (%d)"), static_cast<int32>(ResourceNode->GetResourceAmount()));
			break;
		}

		FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(
			                                              TEXT("%s|%s|%s|%s"),
			                                              *NodeName, *ResourceClass, *ResourceForm, *ResourceAmount
		                                              ), ELogLevel::Debug);
	}

	FResourceRouletteUtilityLog::Get().LogMessage("=== End Logging All AFGResourceNodes in the World ===",
	                                              ELogLevel::Debug);
}
