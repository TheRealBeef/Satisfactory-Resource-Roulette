#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "ResourceNodeSpawner.h"
#include "WorldSeedManager.h"
#include "ResourceRouletteSubsystem.generated.h"

UCLASS()
class RESOURCEROULETTE_API AResourceRouletteSubsystem : public AModSubsystem
{
	GENERATED_BODY()

public:
	static AResourceRouletteSubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	void InitializeResourceRoulette();

	UFUNCTION(BlueprintCallable)
	void UpdateResourceNodes();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool AreResourcesInitialized() const { return bResourcesInitialized; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void SetupWorldSeedManager(UWorld* World);

private:
	bool bResourcesInitialized = false;

	TWeakObjectPtr<AWorldSeedManager> SeedManager;
	UPROPERTY()
	UResourceNodeSpawner* ResourceNodeSpawner;
	FTimerHandle UpdateTimerHandle;
};