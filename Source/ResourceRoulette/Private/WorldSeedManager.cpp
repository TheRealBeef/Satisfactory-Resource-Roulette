#include "WorldSeedManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ResourceNodeUtility.h"

void AWorldSeedManager::SetGlobalSeed(int32 const NewSeed) {
    Seed = NewSeed;
}

int32 AWorldSeedManager::GetGlobalSeed() const {
    return Seed;
}

AWorldSeedManager::AWorldSeedManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AWorldSeedManager::BeginPlay()
{
    Super::BeginPlay();
}

void AWorldSeedManager::OnWorldInitialized(const UWorld::FActorsInitializedParams&)
{
    const int32 CurrentSeed = GetGlobalSeed();
    FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(TEXT("Loaded Seed: %d"), CurrentSeed), ELogLevel::Debug);
}

void AWorldSeedManager::PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
    FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &AWorldSeedManager::OnWorldInitialized);
}

int32 AWorldSeedManager::GenerateSeed()
{
    return FMath::Rand() * FMath::Rand();
}

void AWorldSeedManager::InitRandom()
{
    if (Seed == 0)
    {
        SetGlobalSeed(GenerateSeed());
        FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(TEXT("Generated Seed: %d"), Seed), ELogLevel::Debug);
    }
}

bool AWorldSeedManager::ShouldSave_Implementation() const
{
    return true;
}

void AWorldSeedManager::PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
}

void AWorldSeedManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWorldSeedManager, Seed);
}
