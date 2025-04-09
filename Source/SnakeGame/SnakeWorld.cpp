// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakeWorld.h"
#include "Engine/World.h"

// Sets default values
ASnakeWorld::ASnakeWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WallMeshInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Wall Mesh Instances"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	WallMeshInstances->SetupAttachment(RootComponent);

	InstancedWalls = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedWalls"));

	InstancedWalls->SetupAttachment(RootComponent);

	InstancedFloors = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedFloors"));

	InstancedFloors->SetupAttachment(RootComponent);

	// Here you can get started on creating your level
	/*
	TArray<FString> MapLines;
	
	FString FilePath = FPaths::ProjectContentDir() + "Maps/SnakeMap.txt";

	if (FFileHelper::LoadFileToStringArray(MapLines, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded map with %d lines"), MapLines.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load map!"));
	} 
	*/


	for (int x = 0; x < 5000; x += 200)
	{
		WallMeshInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(x, 200.0f, 0.0f)));
	}
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
	
}

// Called every frame
void ASnakeWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

