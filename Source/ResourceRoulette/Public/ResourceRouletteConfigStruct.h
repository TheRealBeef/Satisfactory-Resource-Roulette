#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "ResourceRouletteConfigStruct.generated.h"

struct FResourceRouletteConfigStruct_RandomizationOptions;
struct FResourceRouletteConfigStruct_GroupingOptions;

USTRUCT(BlueprintType)
struct FResourceRouletteConfigStruct_RandomizationOptions {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool UsePurityExclusion{};

    UPROPERTY(BlueprintReadWrite)
    bool RandSAM{};

    UPROPERTY(BlueprintReadWrite)
    bool RandUranium{};

    UPROPERTY(BlueprintReadWrite)
    bool RandBauxite{};

    UPROPERTY(BlueprintReadWrite)
    bool RandCrude{};

    UPROPERTY(BlueprintReadWrite)
    bool RandFFDirt{};

    UPROPERTY(BlueprintReadWrite)
    bool RandRPThorium{};
};

USTRUCT(BlueprintType)
struct FResourceRouletteConfigStruct_GroupingOptions {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float GroupRadius{};

    UPROPERTY(BlueprintReadWrite)
    bool GroupSAM{};

    UPROPERTY(BlueprintReadWrite)
    bool GroupUranium{};

    UPROPERTY(BlueprintReadWrite)
    bool GroupBauxite{};

    UPROPERTY(BlueprintReadWrite)
    bool GroupCrude{};

    UPROPERTY(BlueprintReadWrite)
    bool GroupFFDirt{};

    UPROPERTY(BlueprintReadWrite)
    bool GroupRPThorium{};
};

/* Struct generated from Mod Configuration Asset '/ResourceRoulette/ResourceRouletteConfig' */
USTRUCT(BlueprintType)
struct FResourceRouletteConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FResourceRouletteConfigStruct_RandomizationOptions RandomizationOptions{};

    UPROPERTY(BlueprintReadWrite)
    FResourceRouletteConfigStruct_GroupingOptions GroupingOptions{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FResourceRouletteConfigStruct GetActiveConfig(UObject* WorldContext) {
        FResourceRouletteConfigStruct ConfigStruct{};
        FConfigId ConfigId{"ResourceRoulette", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FResourceRouletteConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

