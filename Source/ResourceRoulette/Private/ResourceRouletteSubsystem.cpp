#include "ResourceRouletteSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "WorldSeedManager.h"
#include "EngineUtils.h"

AResourceRouletteSubsystem* AResourceRouletteSubsystem::Get(const UObject* WorldContext)
{
    if (!WorldContext) return nullptr;
    return Cast<AResourceRouletteSubsystem>(UGameplayStatics::GetActorOfClass(WorldContext, AResourceRouletteSubsystem::StaticClass()));
}

void AResourceRouletteSubsystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeResourceRoulette();
}

void AResourceRouletteSubsystem::InitializeResourceRoulette()
{
    if (!GetWorld()) return;
    ResourceNodeSpawner = NewObject<UResourceNodeSpawner>(this);
    SetupWorldSeedManager(GetWorld());
    bResourcesInitialized = true;
    constexpr float UpdateInterval = 1.5f;
    GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &AResourceRouletteSubsystem::UpdateResourceNodes, UpdateInterval, true);
    FResourceNodeUtilityLog::Get().LogMessage("Resource Roulette initialized successfully.", ELogLevel::Debug);
}

void AResourceRouletteSubsystem::SetupWorldSeedManager(UWorld* World)
{
    for (TActorIterator<AWorldSeedManager> It(World); It; ++It)
    {
        SeedManager = *It;
        break;
    }
    if (!SeedManager.IsValid())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        SeedManager = World->SpawnActor<AWorldSeedManager>(AWorldSeedManager::StaticClass(), FTransform(FVector()), SpawnParams);
        if (SeedManager.IsValid())
        {
            SeedManager->InitRandom();
        }
    }
}

void AResourceRouletteSubsystem::UpdateResourceNodes()
{
    if (!GEngine || !GetWorld() || !ResourceNodeSpawner || !SeedManager.IsValid())
    {
        FResourceNodeUtilityLog::Get().LogMessage("UpdateResourceNodes aborted: Missing dependencies.", ELogLevel::Error);
        return;
    }
    int32 GlobalSeed = SeedManager->GetGlobalSeed();
    ResourceNodeSpawner->SetSeed(GlobalSeed);
    ResourceNodeSpawner->ScanWorldResourceNodes(GetWorld());
    ResourceNodeSpawner->ReplaceStandardResourceNodesUpdate(GetWorld());

    FResourceNodeUtilityLog::Get().LogMessage("Resource nodes updated successfully.", ELogLevel::Debug);
}

void AResourceRouletteSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    Super::EndPlay(EndPlayReason);
}