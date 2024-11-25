#pragma once

#include "CoreMinimal.h"


class RESOURCEROULETTE_API ResourceRouletteCompatibilityManager
{

public:
	static void RegisterResourceClass(const FName& ClassName, const FName& Tag);
	static void TagExistingActors(UWorld* World);
	static void SetupActorSpawnCallback(UWorld* World);

	static const TArray<FName>& GetRegisteredTags();

	static bool IsCompatibilityClass(AActor* Actor, FName& OutTag);
private:
	static void TagActorAndMesh(AActor* Actor, const FName& Tag);

	static TMap<FName, UClass*> CachedResourceClasses;
	static TMap<FName, FName> CompatResourceClassTags;
	static TArray<FName> RegisteredTags;
	static FDelegateHandle SpawnCallbackHandle;
};
