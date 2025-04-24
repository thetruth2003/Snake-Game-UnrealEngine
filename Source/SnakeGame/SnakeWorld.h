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

	// Root scene component.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USceneComponent* SceneComponent;

	// Instanced mesh component for walls.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInstancedStaticMeshComponent* InstancedWalls;

	// Instanced mesh component for floors.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInstancedStaticMeshComponent* InstancedFloors;

	// Optional door actor class.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AActor> DoorActor;

	// Food actor class to spawn (the apple).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	TSubclassOf<AActor> FoodClass;

	// (Optional) Delay before spawning the next apple.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	float FoodSpawnDelay = 0.5f;

	// Actors spawned as part of level construction (e.g., doors).
	UPROPERTY()
	TArray<AActor*> SpawnedActors;

	// Override OnConstruction to read the level layout and build the world.
	virtual void OnConstruction(const FTransform& Transform) override;
    
	// Spawns a food actor on a random floor tile.
	UFUNCTION()
	void SpawnFood();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	int32 LevelIndex = 1;

	/** 
	 * Clears out the current tile instances and (re)builds walls/floors
	 * from Levels/Level<Index>.txt
	 */
	UFUNCTION(BlueprintCallable, Category="Level")
	void LoadLevelFromText();
	void LoadLevelFromTextOld();

	/** Returns true if Levels/Level<Index>.txt actually exists on disk */
	UFUNCTION(BlueprintCallable, Category="Level")
	bool DoesLevelExist(int32 Index) const;

protected:
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;
	

public:    
	// Called every frame.
	virtual void Tick(float DeltaTime) override;
	const TArray<FVector>& GetFloorTileLocations() const { return FloorTileLocations; }

private:
	// Array holding locations of floor tiles (read from the level file).
	TArray<FVector> FloorTileLocations;
};
