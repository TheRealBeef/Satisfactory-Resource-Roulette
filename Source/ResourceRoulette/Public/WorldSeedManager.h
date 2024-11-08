#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "WorldSeedManager.generated.h"

UCLASS()
class RESOURCEROULETTE_API AWorldSeedManager : public AActor, public IFGSaveInterface
{
	GENERATED_BODY()
	
public:	
	AWorldSeedManager();

	virtual void BeginPlay() override;

	int32 GetGlobalSeed() const;
	void SetGlobalSeed(int32 NewSeed);
	void OnWorldInitialized(const UWorld::FActorsInitializedParams& ActorsInitializedParams);

	static int32 GenerateSeed();
	void InitRandom();

	virtual bool ShouldSave_Implementation() const override;
	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	int32 Seed = 0;
};
