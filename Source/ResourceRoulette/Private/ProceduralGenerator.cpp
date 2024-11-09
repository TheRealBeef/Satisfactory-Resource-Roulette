#include "ProceduralGenerator.h"
#include "Math/UnrealMathUtility.h"

FProceduralGenerator::FProceduralGenerator() {}
FProceduralGenerator::FProceduralGenerator(int32 const InSeed) : Seed(InSeed) {}

int32 FProceduralGenerator::ProceduralRangeIntByVector(const FVector& Input, const int32 Min, const int32 MaxInclusive) const
{
    int32 XHash = FProceduralGeneratorUtils::HashVector(Input);
    int32 CombinedSeed = Seed ^ XHash;
    FRandomStream RandomStream(CombinedSeed);

    return RandomStream.RandRange(Min, MaxInclusive);
}

float FProceduralGenerator::ProceduralRangeFloatByVector(const FVector& Input, const float Min, const float MaxInclusive) const
{
    int32 XHash = FProceduralGeneratorUtils::HashVector(Input);
    int32 CombinedSeed = Seed ^ XHash;
    FRandomStream RandomStream(CombinedSeed);

    return RandomStream.FRandRange(Min, MaxInclusive);
}

int32 FProceduralGenerator::ProceduralRangeIntByFloat(const float Input, const int32 Min, const int32 MaxInclusive) const
{
    int32 InputHash = FProceduralGeneratorUtils::HashFloat(Input);
    int32 CombinedSeed = Seed ^ InputHash;
    FRandomStream RandomStream(CombinedSeed);

    return RandomStream.RandRange(Min, MaxInclusive);
}

float FProceduralGenerator::ProceduralRangeFloatByFloat(const float Input, const float Min, const float MaxInclusive) const
{
    int32 InputHash = FProceduralGeneratorUtils::HashFloat(Input);
    int32 CombinedSeed = Seed ^ InputHash;
    FRandomStream RandomStream(CombinedSeed);

    return RandomStream.FRandRange(Min, MaxInclusive);
}

int32 FProceduralGeneratorUtils::HashVector(const FVector& Input)
{
    int32 XHash = static_cast<int32>(FMath::FloorToInt(Input.X) * 73856093);
    int32 YHash = static_cast<int32>(FMath::FloorToInt(Input.Y) * 19349663);
    int32 ZHash = static_cast<int32>(FMath::FloorToInt(Input.Z) * 83492791);

    return XHash ^ YHash << 1 ^ ZHash << 2;
}

int32 FProceduralGeneratorUtils::HashFloat(const float Input)
{
    union
    {
        float F; int32 I;
    } u;
    u.F = Input;

    int32 FloatBitsHash = u.I ^ (u.I << 13) ^ (u.I >> 17) ^ (u.I << 5);

    int32 MixedHash = static_cast<int32>(FloatBitsHash * 98765.4321f);

    MixedHash ^= (MixedHash << 11);
    MixedHash ^= (MixedHash >> 19);
    MixedHash ^= (MixedHash << 7);

    return MixedHash;
}
