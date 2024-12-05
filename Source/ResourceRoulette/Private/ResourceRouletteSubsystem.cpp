#include "ResourceRouletteSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ResourceRouletteSeedManager.h"
#include "ResourceRouletteUtility.h"
#include "EngineUtils.h"

/// Init the fields on construction or bad things happen
AResourceRouletteSubsystem::AResourceRouletteSubsystem()
{
	ResourceRouletteManager = nullptr;
	SeedManager = nullptr;
	SavedSeed = -1;
	SavedAlreadySpawned = false;
	SavedRandomizedResourceNodes.Empty();
	SavedOriginalResourceNodes.Empty();
	SessionSeed = -1;
	SessionAlreadySpawned = false;
	SessionRandomizedResourceNodes.Empty();
}

AResourceRouletteSubsystem* AResourceRouletteSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}
	return Cast<AResourceRouletteSubsystem>(UGameplayStatics::GetActorOfClass(WorldContext, StaticClass()));
}

/// On BeginPlay, initializes itself
void AResourceRouletteSubsystem::BeginPlay()
{
	Super::BeginPlay();
	InitializeResourceRoulette();
}

/// Initializes subsystem, seed manager, and sets timer to call update method
void AResourceRouletteSubsystem::InitializeResourceRoulette()
{
	if (!GetWorld())
	{
		return;
	}
	ResourceRouletteManager = NewObject<UResourceRouletteManager>(this);

	InitializeWorldSeedManager(GetWorld());
	bIsInitialized = true;
	constexpr float UpdateInterval = 2.5f;
	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &AResourceRouletteSubsystem::UpdateResourceRoulette,
	                                       UpdateInterval, true);
	FResourceRouletteUtilityLog::Get().LogMessage("Resource Roulette initialized successfully.", ELogLevel::Debug);
	UpdateResourceRoulette();
}

/// Called to re-roll Resources
void AResourceRouletteSubsystem::RerollResources()
{
	SessionSeed = -1;
	InitializeWorldSeedManager(GetWorld());
	ResourceRouletteManager->Update(GetWorld(), SeedManager, true);
}

/// Called to update Resources - currently just resets positions back to original and re-lays them down
void AResourceRouletteSubsystem::UpdateResources()
{
	ResourceRouletteManager->Update(GetWorld(), SeedManager, true);
}


/// Prep the mod for removal and destroy miners on our nodes
void AResourceRouletteSubsystem::PrepForRemoval()
{
	ResourceRouletteManager->PrepModForRemoval();
}

/// Checks if seed manager is created, if not it spawns a new one
/// Then checks if there's already a seed from save file. If there's
/// not then SeedManager makes a new one. If there is, then we write
/// this seed to the SeedManager instead.
/// @param World 
void AResourceRouletteSubsystem::InitializeWorldSeedManager(UWorld* World)
{
	for (TActorIterator<AResourceRouletteSeedManager> It(World); It; ++It)
	{
		SeedManager = *It;
		break;
	}
	if (!SeedManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SeedManager = World->SpawnActor<AResourceRouletteSeedManager>(AResourceRouletteSeedManager::StaticClass(),
		                                                              FTransform(FVector()), SpawnParams);
	}
	if (SeedManager)
	{
		if (SessionSeed == -1)
		{
			SeedManager->InitRandom();
			SessionSeed = SeedManager->GetGlobalSeed();
			FResourceRouletteUtilityLog::Get().LogMessage(
				FString::Printf(TEXT("Generated new Global Seed: %d"), SessionSeed), ELogLevel::Debug);
		}
		else
		{
			SeedManager->SetGlobalSeed(SessionSeed);
			FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Set Global Seed: %d"), SessionSeed),
			                                              ELogLevel::Debug);
		}
	}
}

/// Update method, called on timer by subsystem
void AResourceRouletteSubsystem::UpdateResourceRoulette() const
{
	if (!GEngine || !GetWorld() || !ResourceRouletteManager || !SeedManager)
	{
		FResourceRouletteUtilityLog::Get().LogMessage("UpdateResourceRoulette aborted: Missing dependencies.",
		                                              ELogLevel::Error);
		return;
	}
	ResourceRouletteManager->Update(GetWorld(), SeedManager);
	// FResourceRouletteUtilityLog::Get().LogMessage("Resource nodes updated successfully.", ELogLevel::Debug);
}

/// On shutdown it Clears the timers.
/// TODO: - We should also clean up all the other things we were playing with
/// @param EndPlayReason I actually have no idea what this is, but I don't think really need to use it
void AResourceRouletteSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	Super::EndPlay(EndPlayReason);
}

/// Writes our data to the savefile
/// @param SaveVersion 
/// @param GameVersion 
void AResourceRouletteSubsystem::PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
	SavedSeed = SessionSeed;
	SavedAlreadySpawned = SessionAlreadySpawned;
	SavedRandomizedResourceNodes = SessionRandomizedResourceNodes;
	SavedOriginalResourceNodes = OriginalResourceNodes;
}

/// Loads our data from the savefile
/// Perhaps it's better to serialize the actors instead so we don't have to respawn them?
/// @param SaveVersion 
/// @param GameVersion 
void AResourceRouletteSubsystem::PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
	if (SavedSeed != -1)
	{
		SessionSeed = SavedSeed;
		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("PostLoadGame: Loaded Saved Seed: %d"), SessionSeed), ELogLevel::Debug);
	}
	if (SavedAlreadySpawned)
	{
		SessionAlreadySpawned = SavedAlreadySpawned;
		FResourceRouletteUtilityLog::Get().LogMessage(
			TEXT("PostLoadGame: Resource Roulette Previously Spawned Resources"), ELogLevel::Debug);
	}
	if (SavedRandomizedResourceNodes.Num() > 0)
	{
		SessionRandomizedResourceNodes = SavedRandomizedResourceNodes;
		FResourceRouletteUtilityLog::Get().LogMessage(
			TEXT("PostLoadGame: Previously Randomized List of Nodes loaded"), ELogLevel::Debug);
	}
	if (SavedOriginalResourceNodes.Num() > 0)
	{
		OriginalResourceNodes = SavedOriginalResourceNodes;
		FResourceRouletteUtilityLog::Get().LogMessage(
			TEXT("PostLoadGame: Original List of Nodes loaded"), ELogLevel::Debug);
	}
}

void AResourceRouletteSubsystem::SetSessionAlreadySpawned(const bool InSessionAlreadySpawned)
{
	SessionAlreadySpawned = InSessionAlreadySpawned;
}

void AResourceRouletteSubsystem::SetSessionRandomizedResourceNodes(
	const TArray<FResourceNodeData>& InSessionRandomizedResourceNodes)
{
	SessionRandomizedResourceNodes = InSessionRandomizedResourceNodes;
}

void AResourceRouletteSubsystem::SetOriginalResourceNodes(const TArray<FResourceNodeData>& InOriginalResourceNodes)
{
	OriginalResourceNodes = InOriginalResourceNodes;
}
