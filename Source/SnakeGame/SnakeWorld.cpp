// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakeWorld.h"
#include "Engine/World.h"
#include "SnakeFood.h" 

// Sets default values
ASnakeWorld::ASnakeWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//WallMeshInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Wall Mesh Instances"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	//WallMeshInstances->SetupAttachment(RootComponent);

	InstancedWalls = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedWalls"));

	InstancedWalls->SetupAttachment(RootComponent);

	// Configure collision for walls.
	InstancedWalls->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InstancedWalls->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	// Set it to overlap with all channels so that when the snake overlaps the wall,
	// an overlap event is fired.
	InstancedWalls->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	// Tag the component as "Wall" so your SnakePawn can detect it.
	InstancedWalls->ComponentTags.Add(FName("Wall"));

	InstancedFloors = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedFloors"));

	InstancedFloors->SetupAttachment(RootComponent);


	/*
	for (int x = 0; x < 5000; x += 200)
	{
		WallMeshInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(x, 200.0f, 0.0f)));
	}
	*/
}

void ASnakeWorld::OnConstruction(const FTransform& Transform)
{
	UE_LOG(LogTemp, Log, TEXT("OnConstruction Called!"));

	// Clean up
	InstancedFloors->ClearInstances();

	InstancedWalls->ClearInstances();

	for (auto& Actor : SpawnedActors)
	{
		Actor->Destroy();
	}

	SpawnedActors.Empty();

	// Load level
	TArray<FString> Lines;
	FString FilePath = FPaths::ProjectDir() + TEXT("Levels/Level1.txt");

	if (FFileHelper::LoadFileToStringArray(Lines, *FilePath))
	{
		int y = 0;
		for (const FString& Line : Lines)
		{
			for (int x = 0; x < Line.Len(); x++)
			{
				// Need to invert Y and switch X and Y to get the map loaded with X as a forward vector (instead of -Y that is in the text file)
				FTransform Offset = FTransform(FRotator::ZeroRotator, FVector((Lines.Num() - y) * 100, x * 100, 0.0f));
				switch (Line[x])
				{ 
				case '#':
					InstancedWalls->AddInstance(Offset);
					break;
				case 'O':
					// Trapdoor
						break;
				case 'D':
					InstancedFloors->AddInstance(Offset);
					// Door
					if (IsValid(DoorActor))
					{
						AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(DoorActor, Offset, FActorSpawnParameters());
						if (IsValid(SpawnedActor))
						{
							SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

							SpawnedActors.Add(SpawnedActor);
						}
					}
					break;
				case '.':
					InstancedFloors->AddInstance(Offset);
					break;
				}
			}

			y++;
		}
	}
}

// Called when the game starts or when spawned
void ASnakeWorld::BeginPlay()
{
	Super::BeginPlay();

	// Start a recurring timer for food spawn.
	GetWorld()->GetTimerManager().SetTimer(FoodTimerHandle, this, &ASnakeWorld::SpawnFood, FoodSpawnInterval, true);
	
}

// Called every frame
void ASnakeWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASnakeWorld::SpawnFood()
{
	if (FoodClass)
	{
		// Define a random grid location. For example, if your level uses tiles of size 100,
		// choose random multiples of 100 for X and Y.
		int32 RandomX = FMath::RandRange(0, 9);  // Adjust range as needed.
		int32 RandomY = FMath::RandRange(0, 9);
		FVector SpawnLocation = FVector(RandomX * 100.0f, RandomY * 100.0f, 0.0f);

		// Optionally check that the spawn location is not occupied by walls (or the snake).
		GetWorld()->SpawnActor<AActor>(FoodClass, SpawnLocation, FRotator::ZeroRotator);
	}
}

