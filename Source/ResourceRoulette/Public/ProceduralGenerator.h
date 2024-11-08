#pragma once

#include "CoreMinimal.h"

class RESOURCEROULETTE_API FProceduralGeneratorUtils
{
public:
    static int32 HashVector(const FVector& Input);
    static int32 HashFloat(float Input);
};

class RESOURCEROULETTE_API FProceduralGenerator
{
public:
    FProceduralGenerator();
    explicit FProceduralGenerator(int32 InSeed);

    int32 ProceduralRangeIntByVector(const FVector& Input, const int32 Min, const int32 MaxInclusive) const;
    float ProceduralRangeFloatByVector(const FVector& Input, const  float Min, const float MaxInclusive) const;
    int32 ProceduralRangeIntByFloat(const float Input, const int32 Min, const int32 MaxInclusive) const;
    float ProceduralRangeFloatByFloat(const float Input, const float Min, const float MaxInclusive) const;

    static constexpr int32 InvalidSeed = -1;

    void SetSeed(int32 const NewSeed) { Seed = NewSeed; }

private:
    int32 Seed = InvalidSeed;
};
