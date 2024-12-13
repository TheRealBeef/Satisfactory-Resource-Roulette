#include "ResourceRouletteSeedManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ResourceRouletteUtility.h"
#include "ResourceRouletteProfiler.h"

/// Setter for global seed
/// @param NewSeed 
void AResourceRouletteSeedManager::SetGlobalSeed(const int32 NewSeed)
{
	Seed = NewSeed;
}

/// Getter for global seel
/// @return 
int32 AResourceRouletteSeedManager::GetGlobalSeed() const
{
	return Seed;
}

AResourceRouletteSeedManager::AResourceRouletteSeedManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

/// Generates a new global seed for randomization
/// @return Returns seed
int32 AResourceRouletteSeedManager::GenerateSeed()
{
	const int32 GeneratedSeed = FMath::Rand() * FMath::Rand();
	FResourceRouletteUtilityLog::Get().LogMessage(
		FString::Printf(TEXT("Initialized Seed: %d"), GeneratedSeed), ELogLevel::Debug);
	return GeneratedSeed;
}

/// If called, simple generates a new seed and sets the global seed to this value
void AResourceRouletteSeedManager::InitRandom()
{
	SetGlobalSeed(GenerateSeed());
	FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Generated Seed: %d"), Seed), ELogLevel::Debug);
}

/// Not quite sure, but it seems relevant for replication purposes as other mods have it?
/// @param OutLifetimeProps 
void AResourceRouletteSeedManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceRouletteSeedManager, Seed);
}
