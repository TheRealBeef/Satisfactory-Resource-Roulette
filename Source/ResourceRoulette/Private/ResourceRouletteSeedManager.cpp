#include "ResourceRouletteSeedManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ResourceRouletteUtility.h"

void AResourceRouletteSeedManager::SetGlobalSeed(const int32 NewSeed)
{
	Seed = NewSeed;
}

int32 AResourceRouletteSeedManager::GetGlobalSeed() const
{
	return Seed;
}

AResourceRouletteSeedManager::AResourceRouletteSeedManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

int32 AResourceRouletteSeedManager::GenerateSeed()
{
	const int32 GeneratedSeed = FMath::Rand() * FMath::Rand();
	FResourceRouletteUtilityLog::Get().LogMessage(
		FString::Printf(TEXT("Reinitialized Seed: %d in OnWorldInitialized"), GeneratedSeed), ELogLevel::Debug);
	return GeneratedSeed;
}

void AResourceRouletteSeedManager::InitRandom()
{
	SetGlobalSeed(GenerateSeed());
	FResourceRouletteUtilityLog::Get().LogMessage(FString::Printf(TEXT("Generated Seed: %d"), Seed), ELogLevel::Debug);
}

void AResourceRouletteSeedManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceRouletteSeedManager, Seed);
}
