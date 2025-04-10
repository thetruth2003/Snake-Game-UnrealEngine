// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SnakeWorld.generated.h"

UCLASS()
class SNAKEGAME_API ASnakeWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASnakeWorld();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UInstancedStaticMeshComponent* WallMeshInstances;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USceneComponent* SceneComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInstancedStaticMeshComponent* InstancedWalls;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInstancedStaticMeshComponent* InstancedFloors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AActor> DoorActor;

	// Food actor class to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	TSubclassOf<AActor> FoodClass;

	// Spawn interval (in seconds) for food.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	float FoodSpawnInterval = 5.0f;

	UPROPERTY()
	TArray<AActor*> SpawnedActors;

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	FTimerHandle FoodTimerHandle;

	// Function to spawn food.
	UFUNCTION()
	void SpawnFood();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
