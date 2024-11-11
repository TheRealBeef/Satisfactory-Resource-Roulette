#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "ResourceRouletteManager.h"
#include "ResourceRouletteSeedManager.h"
#include "ResourceRouletteSubsystem.generated.h"

UCLASS()
class RESOURCEROULETTE_API AResourceRouletteSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	AResourceRouletteSubsystem();

	static AResourceRouletteSubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	void InitializeResourceRoulette();

	UFUNCTION(BlueprintCallable)
	void UpdateResourceRoulette();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	virtual bool ShouldSave_Implementation() const override { return true; }
	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWorldSeedManager(UWorld* World);

private:
	int32 SessionSeed = -1;

	UPROPERTY(SaveGame)
	int32 SavedSeed;
	bool bIsInitialized = false;
	TWeakObjectPtr<AResourceRouletteSeedManager> SeedManager;
	UPROPERTY()
	UResourceRouletteManager* ResourceRouletteManager;
	FTimerHandle UpdateTimerHandle;
};
