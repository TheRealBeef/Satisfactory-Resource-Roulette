﻿#include "ResourceRouletteUtility.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "FGCharacterPlayer.h"
#include "FGCliffActor.h"
#include "HAL/FileManager.h"
#include "Resources/FGResourceNode.h"
#include "HAL/IConsoleManager.h"
#include "ResourceCollectionManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Resources/FGResourceNode.h"
#include "FGPortableMiner.h"
#include "LandscapeStreamingProxy.h"
#include "ResourceRouletteInvalidNode.h"
#include "ResourceRouletteProfiler.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Buildables/FGBuildableWaterPump.h"
#include "Engine/StaticMeshActor.h"
#include "Equipment/FGResourceScanner.h"
#include "Kismet/GameplayStatics.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "Async/ParallelFor.h"
#include "Buildables/FGBuildableFrackingActivator.h"
#include "Buildables/FGBuildableFrackingExtractor.h"

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
	const int32 CurrentLogLevel = CVarLogLevel.GetValueOnAnyThread();
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


/// Is one of the possible valid resources
/// @param ResourceClassName
/// @return True if it's valid
bool UResourceRouletteUtility::IsValidAllResourceClass(const FName& ResourceClassName)
{
	return !ResourceClassName.IsNone() && AllValidResourceClasses.Contains(ResourceClassName);
}

bool UResourceRouletteUtility::IsValidFilteredResourceClass(const FName& ResourceClassName)
{
	return !ResourceClassName.IsNone() && FilteredValidResourceClasses.Contains(ResourceClassName);
}


/// Checks to see if it's valid resource class and infinite, which should filter out deposits
/// @param ResourceNode 
/// @return 
bool UResourceRouletteUtility::IsValidAllInfiniteResourceNode(const AFGResourceNode* ResourceNode)
{
	if (!ResourceNode || ResourceNode->GetResourceAmount() != EResourceAmount::RA_Infinite)
	{
		return false;
	}

	const FName ResourceClassName = ResourceNode && ResourceNode->GetResourceClass()
		                                ? ResourceNode->GetResourceClass()->GetFName()
		                                : NAME_None;

	return IsValidAllResourceClass(ResourceClassName);
}

bool UResourceRouletteUtility::IsValidFilteredInfiniteResourceNode(const AFGResourceNode* ResourceNode)
{
	if (!ResourceNode || ResourceNode->GetResourceAmount() != EResourceAmount::RA_Infinite)
	{
		return false;
	}

	const FName ResourceClassName = ResourceNode && ResourceNode->GetResourceClass()
		                                ? ResourceNode->GetResourceClass()->GetFName()
		                                : NAME_None;

	return IsValidFilteredResourceClass(ResourceClassName);
}

/// Must be init with UpdateValidResourceClasses
TArray<FName> UResourceRouletteUtility::AllValidResourceClasses;
TArray<FName> UResourceRouletteUtility::FilteredValidResourceClasses;

/// Must be init with UpdateNonGroupableResources
TArray<FName> UResourceRouletteUtility::NonGroupableResources;

/// Updates the ValidResourceClasses array with config options
/// @param SessionSettings Session Settings reference
void UResourceRouletteUtility::UpdateValidResourceClasses(const USessionSettingsManager* SessionSettings)
{
	AllValidResourceClasses = {
		// "Desc_NitrogenGas_C", // This is a fracking node, it's on TODO:
		// "Desc_Geyser_C", // We'll come back to geysers later TODO:
		"Desc_LiquidOil_C",
		// "Desc_Water_C", // This is a fracking node, it's on TODO:
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
		// "Desc_RP_Deanium_C", // This is a fracking node, it's on TODO:
		// "Desc_RP_WaterDamNode_C", // We shouldn't randomize this
		// "Desc_WaterTurbineNode_C", // We shouldn't randomize this
		"Desc_RP_Thorium_C"
	};

	FilteredValidResourceClasses = AllValidResourceClasses;

	if (!SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.RandSAM"))
	{
		FilteredValidResourceClasses.Remove("Desc_SAM_C");
	}

	if (!SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.RandUranium"))
	{
		FilteredValidResourceClasses.Remove("Desc_OreUranium_C");
	}

	if (!SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.RandBauxite"))
	{
		FilteredValidResourceClasses.Remove("Desc_OreBauxite_C");
	}

	if (!SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.RandCrude"))
	{
		FilteredValidResourceClasses.Remove("Desc_LiquidOil_C");
	}
	if (!SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.RandFFDirt"))
	{
		FilteredValidResourceClasses.Remove("Desc_FF_Dirt_Fertilized_C");
		FilteredValidResourceClasses.Remove("Desc_FF_Dirt_C");
		FilteredValidResourceClasses.Remove("Desc_FF_Dirt_Wet_C");
	}
	if (!SessionSettings->GetBoolOptionValue("ResourceRoulette.RandOpt.RandRPThorium"))
	{
		FilteredValidResourceClasses.Remove("Desc_RP_Thorium_C");
	}

	// FString FilteredClassesString;
	// for (const FName& Class : FilteredValidResourceClasses)
	// {
	// 	FilteredClassesString += Class.ToString() + TEXT(", ");
	// }
	// FResourceRouletteUtilityLog::Get().LogMessage(
	// 	FString::Printf(TEXT("Updated FilteredValidResourceClasses: [%s]"), *FilteredClassesString),
	// 	ELogLevel::Debug);
}


/// Returns the filtered valid resource classes to be used for randomization
/// @return Filtered list of Valid Resource Classes based on settings
const TArray<FName>& UResourceRouletteUtility::GetFilteredValidResourceClasses()
{
	return FilteredValidResourceClasses;
}

/// Returns the whole list of valid resource classes so we can collect them
/// @return All the list of valid resource classes
const TArray<FName>& UResourceRouletteUtility::GetAllValidResourceClasses()
{
	return AllValidResourceClasses;
}

/// Updates the NonGroupableResources array with config options
/// This is somewhat inverse logic to the randomization options
/// @param SessionSettings Session Settings reference
void UResourceRouletteUtility::UpdateNonGroupableResources(const USessionSettingsManager* SessionSettings)
{
	NonGroupableResources = {
		"Desc_LiquidOil_C",
		"Desc_SAM_C",
		"Desc_OreBauxite_C",
		"Desc_OreUranium_C",
		"Desc_FF_Dirt_Fertilized_C",
		"Desc_FF_Dirt_C",
		"Desc_FF_Dirt_Wet_C",
		"Desc_RP_Thorium_C"
	};

	if (SessionSettings->GetBoolOptionValue("ResourceRoulette.GroupOpt.GroupSAM") || !SessionSettings->
		GetBoolOptionValue("ResourceRoulette.RandOpt.RandSAM"))
	{
		NonGroupableResources.Remove("Desc_SAM_C");
	}
	if (SessionSettings->GetBoolOptionValue("ResourceRoulette.GroupOpt.GroupUranium") || !SessionSettings->
		GetBoolOptionValue("ResourceRoulette.RandOpt.RandUranium"))
	{
		NonGroupableResources.Remove("Desc_OreUranium_C");
	}
	if (SessionSettings->GetBoolOptionValue("ResourceRoulette.GroupOpt.GroupBauxite") || !SessionSettings->
		GetBoolOptionValue("ResourceRoulette.RandOpt.RandBauxite"))
	{
		NonGroupableResources.Remove("Desc_OreBauxite_C");
	}
	if (SessionSettings->GetBoolOptionValue("ResourceRoulette.GroupOpt.GroupCrude") || !SessionSettings->
		GetBoolOptionValue("ResourceRoulette.RandOpt.RandCrude"))
	{
		NonGroupableResources.Remove("Desc_LiquidOil_C");
	}
	if (SessionSettings->GetBoolOptionValue("ResourceRoulette.GroupOpt.GroupFFDirt") || !SessionSettings->
		GetBoolOptionValue("ResourceRoulette.RandOpt.RandFFDirt"))
	{
		NonGroupableResources.Remove("Desc_FF_Dirt_Fertilized_C");
		NonGroupableResources.Remove("Desc_FF_Dirt_C");
		NonGroupableResources.Remove("Desc_FF_Dirt_Wet_C");
	}
	if (SessionSettings->GetBoolOptionValue("ResourceRoulette.GroupOpt.GroupRPThorium") || !SessionSettings->
		GetBoolOptionValue("ResourceRoulette.RandOpt.RandRPThorium"))
	{
		NonGroupableResources.Remove("Desc_RP_Thorium_C");
	}

	// FString NonGroupableString;
	// for (const FName& Resource : NonGroupableResources)
	// {
	// 	NonGroupableString += Resource.ToString() + TEXT(", ");
	// }
	// FResourceRouletteUtilityLog::Get().LogMessage(
	// 	FString::Printf(TEXT("Updated NonGroupableResources: [%s]"), *NonGroupableString),
	// 	ELogLevel::Debug);
}

/// Returns the current NonGroupableResources array
/// @return NonGroupableResources
const TArray<FName>& UResourceRouletteUtility::GetNonGroupableResources()
{
	return NonGroupableResources;
}

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

/// Raycasts nodes and finds how to rotate them properly to the world.
/// Right now writes back to NodeData directly, should probably use setters
/// to do it better. Raycasting is surprisingly cheap so we're using a lot of points
/// for now
/// @param NodeData The NodeData we're checking
/// @param World World Context
/// @param ResourceNodeActor We have to ignore the actor/mesh when raycasting or we hit ourself
/// @return Returns True if it succeeds, false if fails. Failure should be because there was no
///			world to raycast against so we will try again later
bool UResourceRouletteUtility::CalculateLocationAndRotationForNode(FResourceNodeData& NodeData, const UWorld* World,
                                                                   const AActor* ResourceNodeActor)
{
	RR_PROFILE();
	TArray<FVector> SamplePoints;
	const float Radius = 800.0f; // gives 16m search diameter, which is ~2 foundations.
	const int NumPoints = 50; // How many raycast points to check
	FVector LocationAboveGround = NodeData.Location + FVector(0, 0, 600); // Start 4m above ground

	// Vogel disk for sampling. We can precalculate this instead of doing at runtime if this is a bottleneck
	// but the whole process is surprisingly lightweight. We could probably bump to even more points tbh 
	const float GoldenAngle = 2.39996f;
	for (int i = 0; i < NumPoints; ++i)
	{
		float Distance = Radius * FMath::Sqrt(static_cast<float>(i) / NumPoints);
		float Angle = i * GoldenAngle;

		float X = FMath::Cos(Angle) * Distance;
		float Y = FMath::Sin(Angle) * Distance;

		SamplePoints.Add(LocationAboveGround + FVector(X, Y, 0));
	}

	TArray<FVector> HitPoints;
	FCollisionQueryParams QueryParams;
	if (ResourceNodeActor)
	{
		QueryParams.AddIgnoredActor(ResourceNodeActor);
		QueryParams.AddIgnoredComponent(ResourceNodeActor->FindComponentByClass<UStaticMeshComponent>());
	}

	// for (const FVector& StartPoint : SamplePoints)
	// {
	// 	FVector EndPoint = StartPoint - FVector(0, 0, 1200);
	// 	FHitResult Hit;
	// 	if (World->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, ECC_WorldStatic, QueryParams))
	// 	{
	// 		HitPoints.Add(Hit.ImpactPoint);
	// 	}
	// }

	for (const FVector& StartPoint : SamplePoints)
	{
		FVector EndPoint = StartPoint - FVector(0, 0, 1200);
		TArray<FHitResult> Hits;
		if (World->LineTraceMultiByChannel(Hits, StartPoint, EndPoint, ECC_WorldStatic, QueryParams))
		{
			bool bHitPointAdded = false;
			for (const FHitResult& Hit : Hits)
			{
				if (bHitPointAdded)
				{
					break;
				}
				AActor* HitActor = Hit.GetActor();
				if (!HitActor)
				{
					continue;
				}
				if (HitActor->IsA(ALandscapeStreamingProxy::StaticClass()) || HitActor->IsA(AStaticMeshActor::StaticClass()) || HitActor->IsA(AFGCliffActor::StaticClass()))
				{
					HitPoints.Add(Hit.ImpactPoint);
					bHitPointAdded = true;
				}
				// else
				// {
				// 	FString ClassName = HitActor->GetClass()->GetName();
				// 	FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Ignored Hit Class: %s"), *ClassName), ELogLevel::Warning);
				// }
			}
		}
	}


	if (HitPoints.Num() < NumPoints - 40)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			"Insufficient hit points for calculating node location and rotation.", ELogLevel::Warning);
		return false;
	}

	// Calculate average Z height for elevation adjustment, for now we lerp between the original and this one
	float TotalZ = 0.0f;
	for (const FVector& Point : HitPoints)
	{
		TotalZ += Point.Z;
	}
	float AverageZ = TotalZ / HitPoints.Num();
	NodeData.Location.Z = FMath::Lerp(NodeData.Location.Z, AverageZ, 0.75f);

	// Calculate best-fit plane normal and set rotation
	FVector PlaneNormal = CalculateBestFitPlaneNormal(HitPoints);
	FQuat RotationQuat = FQuat::FindBetweenNormals(FVector::UpVector, PlaneNormal);
	NodeData.Rotation = RotationQuat.Rotator();

	// Mark node as raycasted
	NodeData.IsRayCasted = true;
	return true;
}


/// Uses RANSAC to find a best fit plane. It's overkill, but also nice and this only has to run
/// once per node as you come across it (results are serialized to savefile) and I like nice things
/// @param Points Points to check
/// @return Returns the normal of best plane
FVector UResourceRouletteUtility::CalculateBestFitPlaneNormal(const TArray<FVector>& Points)
{
	RR_PROFILE();
	const int MaxIterations = 50; // How many iterations can we run
	const float DistanceThreshold = 50.0f;
	// How far can our point be off plane and still be "in" the plane - 30cm seems good
	int BestInlierCount = 0;
	FVector BestPlaneNormal = FVector::UpVector;

	for (int i = 0; i < MaxIterations; ++i)
	{
		int Index1 = FMath::RandRange(0, Points.Num() - 1);
		int Index2 = FMath::RandRange(0, Points.Num() - 1);
		int Index3 = FMath::RandRange(0, Points.Num() - 1);
		while (Index2 == Index1)
		{
			Index2 = FMath::RandRange(0, Points.Num() - 1);
		}
		while (Index3 == Index1 || Index3 == Index2)
		{
			Index3 = FMath::RandRange(0, Points.Num() - 1);
		}
		FVector P1 = Points[Index1];
		FVector P2 = Points[Index2];
		FVector P3 = Points[Index3];
		FVector PlaneNormal = FVector::CrossProduct(P2 - P1, P3 - P1).GetSafeNormal();
		int InlierCount = 0;
		for (const FVector& Point : Points)
		{
			float Distance = FMath::Abs(FVector::DotProduct(Point - P1, PlaneNormal));
			if (Distance < DistanceThreshold)
			{
				InlierCount++;
			}
		}
		if (InlierCount > BestInlierCount)
		{
			BestInlierCount = InlierCount;
			BestPlaneNormal = PlaneNormal;
		}
	}

	// We can get inverted normals depending on where we start looking, so fix that
	if (BestPlaneNormal.Z < 0)
	{
		BestPlaneNormal *= -1;
	}
	return BestPlaneNormal;
}

/// Associates Miners with the closest node. Useful for reloading a save since we respawn all the nodes
/// Thanks to Oukibt https://github.com/oukibt/ResourceNodeRandomizer for some of the logic
/// @param World World COntext
/// @param ProcessedNodes List of nodes
/// @param SpawnedResourceNodes GUID keyed pointers to the AFGResourceNodes
void UResourceRouletteUtility::AssociateExtractorsWithNodes(
	UWorld* World,
	const TArray<FResourceNodeData>& ProcessedNodes,
	const TMap<FGuid, AFGResourceNode*>& SpawnedResourceNodes)
{
	RR_PROFILE();
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			"AssociateExtractorsWithNodes aborted: World is invalid.",
			ELogLevel::Error
		);
		return;
	}

	const float MinerAssociationRadius = 700.0f; // 7m
	const float PortableMinerRadius = 1500.0f; // 15m

	// Handle Solid Miners
	for (TActorIterator<AFGBuildableResourceExtractor> It(World); It; ++It)
	{
		AFGBuildableResourceExtractor* ResourceExtractor = *It;

		if (ResourceExtractor->IsA(AFGBuildableWaterPump::StaticClass()) ||
			ResourceExtractor->IsA(AFGBuildableFrackingExtractor::StaticClass()) ||
			ResourceExtractor->IsA(AFGBuildableFrackingActivator::StaticClass()))
		{
			continue;
		}

		AFGResourceNode* ExistingNode = Cast<AFGResourceNode>(ResourceExtractor->mExtractableResource);
		if (ExistingNode)
		{
			const FName ExistingNodeClassName = ExistingNode->GetClass()->GetFName();


			// FString NodeName = ExistingNode->GetName();
			// FString NodeClass = ExistingNode->GetClass()->GetName();
			// FResourceRouletteUtilityLog::Get().LogMessage(
			// 	FString::Printf(TEXT("Existing node association - %s - %s"), *NodeName, *NodeClass),
			// 	ELogLevel::Warning);

			if (ExistingNodeClassName != FName("BP_ResourceNode_C"))
			{
				continue;
			}
		}

		FVector ExtractorLocation = ResourceExtractor->GetActorLocation() - FVector(0.0f, 0.0f, 150.0f);
		AFGResourceNode* ClosestNode = nullptr;
		float ClosestDistance = MinerAssociationRadius;

		for (const FResourceNodeData& NodeData : ProcessedNodes)
		{
			if (AFGResourceNode* Node = SpawnedResourceNodes.FindRef(NodeData.NodeGUID))
			{
				if (!Node || Node->IsOccupied())
				{
					continue;
				}

				if (!ResourceExtractor->IsAllowedOnResource(Node))
				{
					continue;
				}

				float Distance = FVector::Dist(ExtractorLocation, NodeData.Location);
				if (Distance < ClosestDistance)
				{
					ClosestNode = Node;
					ClosestDistance = Distance;
				}
			}
		}

		if (ClosestNode)
		{
			ResourceExtractor->SetExtractableResource(ClosestNode);
		}
		else
		{
			AResourceRouletteInvalidNode* InvalidNode = World->SpawnActor<AResourceRouletteInvalidNode>();
			InvalidNode->InitResource(UResourceRouletteInvalidResource::StaticClass(), EResourceAmount::RA_Infinite,
			                          EResourcePurity::RP_Normal);
			ResourceExtractor->SetResourceNode(InvalidNode);
			ResourceExtractor->mOutputInventory->Empty();
			ResourceExtractor->mCurrentExtractProgress = 0.0f;
		}
	}

	// Handle Portable Miners
	for (TActorIterator<AFGPortableMiner> It(World); It; ++It)
	{
		AFGPortableMiner* PortableMiner = *It;

		FVector MinerLocation = PortableMiner->GetActorLocation();
		AFGResourceNode* ClosestNode = nullptr;

		AFGResourceNode* ExistingNode = Cast<AFGResourceNode>(PortableMiner->mExtractResourceNode);
		if (ExistingNode)
		{
			const FName ExistingNodeClassName = ExistingNode->GetClass()->GetFName();


			// FString NodeName = ExistingNode->GetName();
			// FString NodeClass = ExistingNode->GetClass()->GetName();
			// FResourceRouletteUtilityLog::Get().LogMessage(
			// 	FString::Printf(TEXT("Existing node association - %s - %s"), *NodeName, *NodeClass),
			// 	ELogLevel::Warning);

			if (ExistingNodeClassName != FName("BP_ResourceNode_C"))
			{
				continue;
			}
		}

		float ClosestDistance = PortableMinerRadius;

		for (const FResourceNodeData& NodeData : ProcessedNodes)
		{
			if (AFGResourceNode* Node = SpawnedResourceNodes.FindRef(NodeData.NodeGUID))
			{
				float Distance = FVector::Dist(MinerLocation, NodeData.Location);
				if (Distance < ClosestDistance)
				{
					ClosestNode = Node;
					ClosestDistance = Distance;
				}
			}
		}

		if (ClosestNode)
		{
			PortableMiner->mExtractResourceNode = ClosestNode;
		}
		else
		{
			FResourceRouletteUtilityLog::Get().LogMessage(
				FString::Printf(TEXT("Can't find a matching node - destroying portable miner at location: %s"),
				                *MinerLocation.ToString()), ELogLevel::Warning);
			AResourceRouletteInvalidNode* InvalidNode = World->SpawnActor<AResourceRouletteInvalidNode>();
			InvalidNode->InitResource(UResourceRouletteInvalidResource::StaticClass(), EResourceAmount::RA_Infinite,
			                          EResourcePurity::RP_Normal);
			PortableMiner->mExtractResourceNode = InvalidNode;
			PortableMiner->mOutputInventory->Empty();
			PortableMiner->mCurrentExtractProgress = 0.0f;
		}
	}
}

/// Removes all extractors that are located on the custom nodes.
/// @param World World Context
/// @param ProcessedNodes List of nodes
/// @param SpawnedResourceNodes GUID keyed pointers to the AFGResourceNodes
void UResourceRouletteUtility::RemoveExtractors(
	UWorld* World,
	const TArray<FResourceNodeData>& ProcessedNodes,
	const TMap<FGuid, AFGResourceNode*>& SpawnedResourceNodes)
{
	RR_PROFILE();
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage(
			"RemoveExtractors aborted: World is invalid.",
			ELogLevel::Error
		);
		return;
	}

	//AFGBuildableResourceExtractor
	for (TActorIterator<AFGBuildableResourceExtractor> It(World); It; ++It)
	{
		AFGBuildableResourceExtractor* ResourceExtractor = *It;

		if (ResourceExtractor->IsA(AFGBuildableWaterPump::StaticClass()))
		{
			continue;
		}

		AFGResourceNode* ExtractorNode = Cast<AFGResourceNode>(ResourceExtractor->mExtractableResource);
		if (ExtractorNode)
		{
			for (const auto& Pair : SpawnedResourceNodes)
			{
				if (Pair.Value == ExtractorNode)
				{
					ResourceExtractor->Destroy();
					break;
				}
			}
		}
	}

	// AFGPortableMiner
	for (TActorIterator<AFGPortableMiner> It(World); It; ++It)
	{
		AFGPortableMiner* PortableMiner = *It;

		AFGResourceNode* MinerNode = Cast<AFGResourceNode>(PortableMiner->mExtractResourceNode);
		if (MinerNode)
		{
			for (const auto& Pair : SpawnedResourceNodes)
			{
				if (Pair.Value == MinerNode)
				{
					PortableMiner->Destroy();
					break;
				}
			}
		}
	}
}


void UResourceRouletteUtility::ScannerGenerateNodeClusters(UWorld* World, float ClusterRadius)
{
	RR_PROFILE();
	if (!World)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("ScannerGenerateNodeClusters: Invalid World!",
											  ELogLevel::Warning);
		return;
	}

	// Grab resource scanner (may need different method in Multiplayer? TODO:)
	AFGResourceScanner* ResourceScanner = Cast<AFGResourceScanner>(
		UGameplayStatics::GetActorOfClass(World, AFGResourceScanner::StaticClass()));
	if (!ResourceScanner)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("ScannerGenerateNodeClusters: No Resource Scanner Found.",
		                                              ELogLevel::Warning);
		return;
	}

	TArray<AFGResourceNodeBase*> ResourceNodes;
	for (TActorIterator<AFGResourceNodeBase> It(World); It; ++It)
	{
		AFGResourceNodeBase* BaseNode = *It;
		if (BaseNode)
		{
			AFGResourceNode* ResourceNode = Cast<AFGResourceNode>(BaseNode);
			if (ResourceNode && ResourceNode->GetResourceAmount() == EResourceAmount::RA_Infinite)
			{
				ResourceNodes.Add(BaseNode);
			}
		}
	}

	if (ResourceNodes.Num() == 0)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("ScannerGenerateNodeClusters: No resource nodes found.",
		                                              ELogLevel::Warning);
		return;
	}

	// Group nodes by resource class
	TMap<TSubclassOf<UFGResourceDescriptor>, TArray<AFGResourceNodeBase*>> NodesByResourceClass;
	for (AFGResourceNodeBase* Node : ResourceNodes)
	{
		if (Node && Node->GetResourceClass())
		{
			NodesByResourceClass.FindOrAdd(Node->GetResourceClass()).Add(Node);
		}
	}

	TArray<TArray<AFGResourceNodeBase*>> ResourceClassBuckets;
	const int32 NumThreads = NodesByResourceClass.Num();
	ResourceClassBuckets.SetNum(NumThreads);

	// Originally wanted to group smaller resource nodes on a single thread to max out utilization but this may be unnecessary
	NodesByResourceClass.ValueSort([](const TArray<AFGResourceNodeBase*>& A, const TArray<AFGResourceNodeBase*>& B)
	{
		return A.Num() > B.Num();
	});

	int32 ThreadIndex = 0;
	for (const auto& Pair : NodesByResourceClass)
	{
		ResourceClassBuckets[ThreadIndex].Append(Pair.Value);
		ThreadIndex = (ThreadIndex + 1) % NumThreads;
	}

	// Clustering nodes
	TArray<FNodeClusterData> NodeClusters;
	FCriticalSection CriticalSection;

	ParallelFor(NodesByResourceClass.Num(), [&](int32 Index)
	{
		TArray<AFGResourceNodeBase*> Nodes = NodesByResourceClass.Array()[Index].Value;
		TArray<FNodeClusterData> ThreadClusters;

		while (Nodes.Num() > 0)
		{
			AFGResourceNodeBase* Node = Nodes.Pop();
			TArray<AFGResourceNodeBase*> ClusterNodes;
			ClusterNodes.Add(Node);

			FVector ClusterMidPoint = Node->GetActorLocation();

			for (int32 i = Nodes.Num() - 1; i >= 0; --i)
			{
				if (FVector::Dist(ClusterMidPoint, Nodes[i]->GetActorLocation()) <= ClusterRadius)
				{
					ClusterNodes.Add(Nodes[i]);
					Nodes.RemoveAtSwap(i);
				}
			}

			FVector Sum = FVector::ZeroVector;
			for (AFGResourceNodeBase* ClusterNode : ClusterNodes)
			{
				Sum += ClusterNode->GetActorLocation();
			}
			ClusterMidPoint = Sum / ClusterNodes.Num();

			FNodeClusterData ThreadClusterData;
			ThreadClusterData.Nodes = ClusterNodes;
			ThreadClusterData.MidPoint = ClusterMidPoint;
			ThreadClusterData.ResourceDescriptor = Node->GetResourceClass();

			ThreadClusters.Add(ThreadClusterData);
		}

		// Add to the big bucket of clusters
		FScopeLock Lock(&CriticalSection);
		for (const FNodeClusterData& Cluster : ThreadClusters)
		{
			if (Cluster.Nodes.Num() > 0)
			{
				NodeClusters.Add(Cluster);
			}
		}
	});

	ResourceScanner->mNodeClusters = NodeClusters;

	// TMap<TSubclassOf<UFGResourceDescriptor>, int32> ClusterCountsByResourceClass;
	// for (const FNodeClusterData& Cluster : NodeClusters)
	// {
	// 	if (Cluster.ResourceDescriptor)
	// 	{
	// 		ClusterCountsByResourceClass.FindOrAdd(Cluster.ResourceDescriptor)++;
	// 	}
	// }
	//
	// FString ResourceClassDetails;
	// for (const auto& Pair : ClusterCountsByResourceClass)
	// {
	// 	if (Pair.Key)
	// 	{
	// 		FString ResourceClassName = Pair.Key->GetName();
	// 		int32 ClusterCount = Pair.Value;
	// 		ResourceClassDetails += FString::Printf(TEXT("%s: %d clusters\n"), *ResourceClassName, ClusterCount);
	// 	}
	// }
	//
	// FResourceRouletteUtilityLog::Get().LogMessage(
	// FString::Printf(TEXT("ScannerGenerateNodeClusters: Generated %d clusters across %d threads.\n%s"),
	// 				NodeClusters.Num(), NumThreads, *ResourceClassDetails),
	// ELogLevel::Warning);

	// FResourceRouletteUtilityLog::Get().LogMessage(
		// FString::Printf(TEXT("ScannerGenerateNodeClusters: Generated %d clusters across %d threads"),
		                // NodeClusters.Num(), NumThreads), ELogLevel::Warning);
}
