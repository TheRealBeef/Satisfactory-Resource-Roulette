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
	void RerollResources();
	
	UFUNCTION(BlueprintCallable)
	void UpdateResources();

	UFUNCTION(BlueprintCallable)
	void UpdateResourceRoulette() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	bool GetSessionAlreadySpawned() const { return SessionAlreadySpawned; }
	TArray<FResourceNodeData>& GetSessionRandomizedResourceNodes() { return SessionRandomizedResourceNodes; }
	TArray<FResourceNodeData>& GetOriginalResourceNodes() { return OriginalResourceNodes; }

	void SetSessionAlreadySpawned(bool InSessionAlreadySpawned);
	void SetSessionRandomizedResourceNodes(const TArray<FResourceNodeData>& InSessionRandomizedResourceNodes);
	void SetOriginalResourceNodes(const TArray<FResourceNodeData>& InOriginalResourceNodes);
	
	virtual bool ShouldSave_Implementation() const override { return true; }
	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWorldSeedManager(UWorld* World);

private:
	UPROPERTY(SaveGame)
	int32 SavedSeed;
	int32 SessionSeed = -1;

	UPROPERTY(SaveGame)
	bool SavedAlreadySpawned = false;;
	bool SessionAlreadySpawned = false;

	UPROPERTY(SaveGame)
	TArray<FResourceNodeData> SavedRandomizedResourceNodes;

	UPROPERTY(SaveGame)
	TArray<FResourceNodeData> SavedOriginalResourceNodes;

	UPROPERTY()
	TArray<FResourceNodeData> SessionRandomizedResourceNodes;

	UPROPERTY()
	TArray<FResourceNodeData> OriginalResourceNodes;

	bool bIsInitialized = false;

	UPROPERTY()
	AResourceRouletteSeedManager* SeedManager;

	UPROPERTY()
	UResourceRouletteManager* ResourceRouletteManager;

	FTimerHandle UpdateTimerHandle;
};
