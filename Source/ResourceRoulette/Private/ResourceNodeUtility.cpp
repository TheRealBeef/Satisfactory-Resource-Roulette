#include "ResourceNodeUtility.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "FGResourceNode.h"
#include "HAL/IConsoleManager.h"
#include "ResourceNodeAssets.h"

// UE_DISABLE_OPTIMIZATION
DEFINE_LOG_CATEGORY_STATIC(LogResourceRoulette, Log, All);

static TAutoConsoleVariable<int32> CVarLogLevel(
	TEXT("ResourceRoulette.LogLevel"), 1,
	TEXT("Sets log verbosity level: 0 = Error, 1 = Warning, 2 = Debug"),
	ECVF_Default
);

FResourceNodeUtilityLog FResourceNodeUtilityLog::Instance;
FResourceNodeUtilityLog& FResourceNodeUtilityLog::Get()
{
	return Instance;
}

void FResourceNodeUtilityLog::InitializeLog()
{
	FScopeLock Lock(&LogFileMutex);
	if (!LogFile)
	{
		FString LogFilePath = FPaths::Combine(
			FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA")),
			TEXT("FactoryGame/Saved/Logs/ResourceRoulette.log")
		);
		LogFile = new FOutputDeviceFile(*LogFilePath, true);
		LogFile->SetAutoEmitLineTerminator(true);
		GLog->AddOutputDevice(this);
		LogMessage("Resource Roulette Module Log Startup", ELogLevel::Debug);
	}
}


void FResourceNodeUtilityLog::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (Category == TEXT("LogResourceRoulette"))
	{
		FString LogMessage(V);
		if (LogFile)
		{
			LogFile->Serialize(*LogMessage, Verbosity, TEXT("")); // Pass an empty category name
		}
	}
}

void FResourceNodeUtilityLog::ShutdownLog()
{
	FScopeLock Lock(&LogFileMutex);
	if (LogFile)
	{
		LogMessage("Resource Roulette Module Log Shutdown", ELogLevel::Debug);
		GLog->RemoveOutputDevice(this);
		delete LogFile;
		LogFile = nullptr;
	}
}

void FResourceNodeUtilityLog::LogMessage(const FString& Message, ELogLevel Level)
{
	int32 CurrentLogLevel = CVarLogLevel.GetValueOnAnyThread();
	if (static_cast<int32>(Level) >= CurrentLogLevel)
	{
		FScopeLock Lock(&LogFileMutex);
		FString FormattedMessage = Message;

		switch (Level)
		{
		case ELogLevel::Error:
			Serialize(*FormattedMessage, ELogVerbosity::Error, FName("LogResourceRoulette"));
			break;
		case ELogLevel::Warning:
			Serialize(*FormattedMessage, ELogVerbosity::Warning, FName("LogResourceRoulette"));
			break;
		case ELogLevel::Debug:
		default:
			Serialize(*FormattedMessage, ELogVerbosity::Log, FName("LogResourceRoulette"));
			break;
		}
	}
}


void UResourceNodeUtility::InitializeLoggingModule()
{
	FResourceNodeUtilityLog::Get().InitializeLog();
	FResourceNodeUtilityLog::Get().LogMessage("Logging initialized", ELogLevel::Debug);
}

bool UResourceNodeUtility::TryGetOreTypeFromName(const FString& Name, EOreType& OreType)
{
	for (auto& OrePair : OreTypeNameList)
	{
		if (Name.Equals(OrePair.Key, ESearchCase::IgnoreCase))
		{
			OreType = OrePair.Value;
			return true;
		}
	}
	return false;
}

bool UResourceNodeUtility::TryGetNameFromOreType(const EOreType OreType, FString& Name)
{
	for (auto& OrePair : OreTypeNameList)
	{
		if (OreType == OrePair.Value)
		{
			Name = OrePair.Key;
			return true;
		}
	}
	return false;
}

bool UResourceNodeUtility::FindOreTypeFromPath(const FString& Path, EOreType& OutOreType)
{
	for (const auto& OrePair : OreTypeNameList)
	{
		if (Path.Contains(OrePair.Key, ESearchCase::IgnoreCase))
		{
			OutOreType = OrePair.Value;
			return true;
		}
	}
	return false;
}

bool UResourceNodeUtility::GetOreTypeFromResourceNode(const AFGResourceNode* ResourceNode, EOreType& OutOreType)
{
	if (!ResourceNode)
	{
		return false;
	}

	// Check texture path first
	UTexture2D* CompasTexture = UFGResourceDescriptor::GetCompasTexture(ResourceNode->GetResourceClass());
	if (CompasTexture && FindOreTypeFromPath(CompasTexture->GetPathName(), OutOreType))
	{
		return true;
	}
	return FindOreTypeFromPath(ResourceNode->GetPathName(), OutOreType);
}

void UResourceNodeUtility::LogAllResourceNodes(const UWorld* World)
{
	FResourceNodeUtilityLog::Get().LogMessage("=== Begin Logging All AFGResourceNodes in the World ===",ELogLevel::Debug);

	for (TActorIterator<AFGResourceNode> It(World); It; ++It)
	{
		AFGResourceNode* ResourceNode = *It;

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

		FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(
			TEXT("%s|%s|%s|%s"),
			*NodeName, *ResourceClass, *ResourceForm, *ResourceAmount
		), ELogLevel::Debug);
	}

	FResourceNodeUtilityLog::Get().LogMessage("=== End Logging All AFGResourceNodes in the World ===", ELogLevel::Debug);
}

// UE_ENABLE_OPTIMIZATION