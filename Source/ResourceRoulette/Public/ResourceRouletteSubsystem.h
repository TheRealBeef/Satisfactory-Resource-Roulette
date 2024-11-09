#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "ResourceNodeSpawner.h"
#include "WorldSeedManager.h"
#include "ResourceRouletteSubsystem.generated.h"

UCLASS()
class RESOURCEROULETTE_API AResourceRouletteSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	AResourceRouletteSubsystem() : SavedSeed(-1), ResourceNodeSpawner(nullptr)	{}

	static AResourceRouletteSubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	void InitializeResourceRoulette();

	UFUNCTION(BlueprintCallable)
	void UpdateResourceNodes();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool AreResourcesInitialized() const { return bResourcesInitialized; }
	
	virtual bool ShouldSave_Implementation() const override { return true; }
	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void SetupWorldSeedManager(UWorld* World);

private:
	int32 SessionSeed = -1;
	
	UPROPERTY(SaveGame)
	int32 SavedSeed;
	bool bResourcesInitialized = false;
	TWeakObjectPtr<AWorldSeedManager> SeedManager;
	UPROPERTY()
	UResourceNodeSpawner* ResourceNodeSpawner;
	FTimerHandle UpdateTimerHandle;
};