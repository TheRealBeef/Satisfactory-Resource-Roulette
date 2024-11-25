#include "ResourceRouletteCompatibilityManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "ResourceRouletteUtility.h"
#include "EngineUtils.h"

TMap<FName, UClass*> ResourceRouletteCompatibilityManager::CachedResourceClasses;
TMap<FName, FName> ResourceRouletteCompatibilityManager::CompatResourceClassTags;
TArray<FName> ResourceRouletteCompatibilityManager::RegisteredTags;
FDelegateHandle ResourceRouletteCompatibilityManager::SpawnCallbackHandle;

/// Adds resource class and tag for compatibility with other mods
/// @param ClassName Classname to add
/// @param Tag Tag to give actors/meshes of this class
void ResourceRouletteCompatibilityManager::RegisterResourceClass(const FName& ClassName, const FName& Tag)
{
    CompatResourceClassTags.Add(ClassName, Tag);

    if (!RegisteredTags.Contains(Tag))
    {
        RegisteredTags.Add(Tag);
    }
}

/// Tag the existing actors/meshes in the world - to be used on init
/// @param World World Context
void ResourceRouletteCompatibilityManager::TagExistingActors(UWorld* World)
{
    if (!World)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            TEXT("TagExistingActors aborted: World is null."),
            ELogLevel::Warning
        );
        return;
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* ExistingActor = *It;
        FName Tag;
        if (IsCompatibilityClass(ExistingActor, Tag))
        {
            TagActorAndMesh(ExistingActor, Tag);
        }
    }
}

/// Adds a callback for actor spawns ... TODO make this as light as possible since we're hooking into all the spawns :#
/// @param World 
void ResourceRouletteCompatibilityManager::SetupActorSpawnCallback(UWorld* World)
{
    if (!SpawnCallbackHandle.IsValid())
    {
        SpawnCallbackHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([](AActor* SpawnedActor) {
            if (SpawnedActor)
            {
                FName Tag;
                if (IsCompatibilityClass(SpawnedActor, Tag))
                {
                    TagActorAndMesh(SpawnedActor, Tag);
                }
            }
        }));

        FResourceRouletteUtilityLog::Get().LogMessage(
            TEXT("Persistent callback for tagging new actors has been registered."),
            ELogLevel::Debug
        );
    }
}

/// Tag the actor and the mesh with the custom tags
/// @param Actor Actor to tag
/// @param Tag Tag to add
void ResourceRouletteCompatibilityManager::TagActorAndMesh(AActor* Actor, const FName& Tag)
{
    if (!Actor->Tags.Contains(Tag))
    {
        Actor->Tags.Add(Tag);
    }

    TArray<UActorComponent*> Components = Actor->GetComponents().Array();
    for (UActorComponent* Component : Components)
    {
        if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component))
        {
            if (!MeshComp->ComponentTags.Contains(Tag))
            {
                MeshComp->ComponentTags.Add(Tag);
                // FResourceRouletteUtilityLog::Get().LogMessage(
                //     FString::Printf(TEXT("Tagged Mesh Component: %s of Actor: %s with Tag: %s"),
                //         *MeshComp->GetName(), *Actor->GetName(), *Tag.ToString()),
                //     ELogLevel::Debug
                // );
            }
        }
    }
}

/// Check to see if it's one of the classes we should be caring about
/// @param Actor Actor to check
/// @param OutTag Output tag
/// @return Returns true on success false if its not compatible
bool ResourceRouletteCompatibilityManager::IsCompatibilityClass(AActor* Actor, FName& OutTag)
{
    if (!Actor)
    {
        FResourceRouletteUtilityLog::Get().LogMessage(
            TEXT("IsCompatibilityClass aborted: Actor is null."),
            ELogLevel::Warning
        );
        return false;
    }

    UClass* ActorClass = Actor->GetClass();
    FString ActorClassName = ActorClass->GetName();
   
    // // the actor being checked
    // FResourceRouletteUtilityLog::Get().LogMessage(
    //     FString::Printf(TEXT("Checking Actor: %s with Class: %s"), *Actor->GetName(), *ActorClassName),
    //     ELogLevel::Debug
    // );

    // // log the class hierarchy
    // FResourceRouletteUtilityLog::Get().LogMessage(
    //     FString::Printf(TEXT("Class Hierarchy for Actor: %s"), *Actor->GetName()),
    //     ELogLevel::Debug
    // );
    //
    // UClass* CurrentClass = ActorClass;
    // while (CurrentClass)
    // {
    //     FResourceRouletteUtilityLog::Get().LogMessage(
    //         FString::Printf(TEXT("- %s"), *CurrentClass->GetName()),
    //         ELogLevel::Debug
    //     );
    //     CurrentClass = CurrentClass->GetSuperClass();
    // }
    
    for (const auto& Pair : CompatResourceClassTags)
    {
        const FName& RegisteredClassName = Pair.Key;
        const FName& Tag = Pair.Value;
        
        UClass* RegisteredClass = CachedResourceClasses.FindRef(RegisteredClassName);
        if (!RegisteredClass)
        {
            FString ClassToFind = RegisteredClassName.ToString();
            RegisteredClass = FindObject<UClass>(ANY_PACKAGE, *ClassToFind);
            if (!RegisteredClass && !ClassToFind.StartsWith(TEXT("A")))
            {
                FString PrefixedClassName = TEXT("A") + ClassToFind;
                RegisteredClass = FindObject<UClass>(ANY_PACKAGE, *PrefixedClassName);
            }

            if (RegisteredClass)
            {
                CachedResourceClasses.Add(RegisteredClassName, RegisteredClass);
            }
            else
            {
                continue;
            }
        }
        if (RegisteredClass && ActorClass->IsChildOf(RegisteredClass))
        {
            OutTag = Tag;
            return true;
        }
    }

    return false;
}


const TArray<FName>& ResourceRouletteCompatibilityManager::GetRegisteredTags()
{
    return RegisteredTags;
}
