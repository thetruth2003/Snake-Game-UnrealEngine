// SnakeWorld.cpp

#include "SnakeWorld.h"
#include "Engine/World.h"
#include "SnakeFood.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Sets default values
ASnakeWorld::ASnakeWorld()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and set up the root scene component.
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

    // Create and set up the wall instances.
    InstancedWalls = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedWalls"));
    InstancedWalls->SetupAttachment(RootComponent);
    // Hard override: Use QueryOnly collision and Overlap responses.
    InstancedWalls->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InstancedWalls->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    InstancedWalls->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    // Ensure the "Wall" tag is present.
    InstancedWalls->ComponentTags.Empty();
    InstancedWalls->ComponentTags.Add(FName("Wall"));

    // Create and set up the floor instances.
    InstancedFloors = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedFloors"));
    InstancedFloors->SetupAttachment(RootComponent);
}

void ASnakeWorld::OnConstruction(const FTransform& Transform)
{
    UE_LOG(LogTemp, Log, TEXT("OnConstruction Called!"));

    // Clean up previous instances and spawned actors.
    InstancedWalls->ClearInstances();
    InstancedFloors->ClearInstances();
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    SpawnedActors.Empty();
    
    // Clear previous floor tile locations.
    FloorTileLocations.Empty();

    // Reapply collision settings to override any Blueprint changes.
    InstancedWalls->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InstancedWalls->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    InstancedWalls->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    if (!InstancedWalls->ComponentTags.Contains(FName("Wall")))
    {
        InstancedWalls->ComponentTags.Empty();
        InstancedWalls->ComponentTags.Add(FName("Wall"));
    }

    // Load level layout from a text file.
    TArray<FString> Lines;
    FString FilePath = FPaths::ProjectDir() + TEXT("Levels/Level1.txt");

    if (FFileHelper::LoadFileToStringArray(Lines, *FilePath))
    {
        int y = 0;
        // Loop through each line of the file.
        for (const FString& Line : Lines)
        {
            for (int x = 0; x < Line.Len(); x++)
            {
                // Calculate the transform for the current tile.
                // Here (Lines.Num() - y)*100 makes X the forward direction.
                FTransform TileTransform(FRotator::ZeroRotator, FVector((Lines.Num() - y) * 100, x * 100, 0.0f));
                
                // Process each character in the level file.
                switch (Line[x])
                {
                    case '#':
                        // Wall tile.
                        InstancedWalls->AddInstance(TileTransform);
                        break;

                    case 'O':
                        // Trapdoor (optional logic).
                        break;

                    case 'D':
                        // Door tile: add floor instance and optionally spawn door actor.
                        InstancedFloors->AddInstance(TileTransform);
                        if (IsValid(DoorActor))
                        {
                            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(DoorActor, TileTransform, FActorSpawnParameters());
                            if (SpawnedActor)
                            {
                                SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
                                SpawnedActors.Add(SpawnedActor);
                            }
                        }
                        break;

                    case '.':
                        // Valid floor tile: add instance and record the position.
                        InstancedFloors->AddInstance(TileTransform);
                        FloorTileLocations.Add(TileTransform.GetTranslation());
                        break;
                }
            }
            y++;
        }
    }
}

void ASnakeWorld::BeginPlay()
{
    Super::BeginPlay();

    // Spawn the initial apple.
    SpawnFood();
}

void ASnakeWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASnakeWorld::SpawnFood()
{
    // Check if a FoodClass is set and there is at least one valid floor tile.
    if (FoodClass && FloorTileLocations.Num() > 0)
    {
        // Pick a random floor tile.
        int32 RandomIndex = FMath::RandRange(0, FloorTileLocations.Num() - 1);
        int32 RandomIndex2 = FMath::RandRange(0, FloorTileLocations.Num() - 1);
        FVector SpawnLocation = GetActorLocation() + FloorTileLocations[RandomIndex];

        // Spawn the food actor (apple) at the chosen location.
        GetWorld()->SpawnActor<AActor>(FoodClass, SpawnLocation, FRotator::ZeroRotator);
    }
}
