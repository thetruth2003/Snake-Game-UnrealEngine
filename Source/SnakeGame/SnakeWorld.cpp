// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakeWorld.h"

// Sets default values
ASnakeWorld::ASnakeWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WallMeshInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Wall Mesh Instances"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	WallMeshInstances->SetupAttachment(RootComponent);

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

