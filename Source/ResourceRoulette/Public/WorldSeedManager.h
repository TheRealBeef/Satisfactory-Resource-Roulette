#pragma once

#include "CoreMinimal.h"
#include "WorldSeedManager.generated.h"

UCLASS()
class RESOURCEROULETTE_API AWorldSeedManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWorldSeedManager();

	int32 GetGlobalSeed() const;
	void SetGlobalSeed(int32 NewSeed);

	static int32 GenerateSeed();
	void InitRandom();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	int32 Seed = -1;
};
