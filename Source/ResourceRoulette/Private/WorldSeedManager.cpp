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

    int32 AWorldSeedManager::GenerateSeed()
    {
        int32 GeneratedSeed = FMath::Rand() * FMath::Rand();
        FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(TEXT("Reinitialized Seed: %d in OnWorldInitialized"), GeneratedSeed), ELogLevel::Debug);
        return GeneratedSeed;
    }

    void AWorldSeedManager::InitRandom()
    {
        SetGlobalSeed(GenerateSeed());
        FResourceNodeUtilityLog::Get().LogMessage(FString::Printf(TEXT("Generated Seed: %d"), Seed), ELogLevel::Debug);
    }

    void AWorldSeedManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);

        DOREPLIFETIME(AWorldSeedManager, Seed);
    }
