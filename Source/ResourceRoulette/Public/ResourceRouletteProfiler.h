#pragma once

#include "CoreMinimal.h"
#include "ResourceRouletteUtility.h"
#include "Windows/WindowsPlatformTime.h"
//////////////////////////////////////////////////////////////////////////////////
// If this is 1 then we do profiling, if it's 0 then we don't do profiling      //
// SET TO ZERO BEFORE RELEASING MOD												//
//////////////////////////////////////////////////////////////////////////////////
#define ENABLE_PROFILING 1

#if ENABLE_PROFILING
#define RR_PROFILE() FResourceRouletteProfiler ScopedProfiler(TEXT(__FUNCTION__))
#define RR_PROFILE_SUBSECTION(SubsectionName) FResourceRouletteProfilerSubsection ScopedSubProfiler(TEXT(__FUNCTION__), TEXT(SubsectionName))
#else
#define RR_PROFILE()
#define RR_PROFILE_SUBSECTION(SubsectionName)
#endif


class FResourceRouletteProfiler
{
public:
	explicit FResourceRouletteProfiler(const FString& InFunctionName) : FunctionName(InFunctionName)
	{
		StartTime = FPlatformTime::Seconds();
	}

	~FResourceRouletteProfiler()
	{
		const double EndTime = FPlatformTime::Seconds();
		const double ElapsedTime = (EndTime - StartTime) * 1000.0;

		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("%s executed in %.3f ms"), *FunctionName, ElapsedTime),
			ELogLevel::Debug);
	}

private:
	double StartTime;
	FString FunctionName;
};

class FResourceRouletteProfilerSubsection
{
public:
	FResourceRouletteProfilerSubsection(const TCHAR* InFunctionName, const TCHAR* InSubsectionName)
	{
		StartTime = FPlatformTime::Seconds();
		SubFunctionName = FString::Printf(TEXT("%s - %s"), InFunctionName, InSubsectionName);
	}

	~FResourceRouletteProfilerSubsection()
	{
		const double EndTime = FPlatformTime::Seconds();
		const double ElapsedTime = (EndTime - StartTime) * 1000.0;

		FResourceRouletteUtilityLog::Get().LogMessage(
			FString::Printf(TEXT("%s executed in %.3f ms"), *SubFunctionName, ElapsedTime),
			ELogLevel::Debug);
	}

private:
	double StartTime;
	FString SubFunctionName;
};