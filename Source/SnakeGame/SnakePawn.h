// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "SnakePawn.generated.h"

class ASnakeTailSegment;

UCLASS()
class SNAKEGAME_API ASnakePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASnakePawn();

	// So that we can have multiple children connected to the root component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	// Our snake collider
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESnakeDirection Direction = ESnakeDirection::None;

	// Array holding tail segment actors.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snake")
	TArray<ASnakeTailSegment*> TailSegments;

	// Tracks the snake's tile position (used when moving from one tile to the next).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snake")
	FVector LastTilePosition;

	// Call to add a tail segment when food is eaten.
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void GrowTail();
	void UpdateTailTargets(const FVector& PreviousHeadPosition);

	// Called after a full tile movement to update positions of each tail segment.
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void UpdateTailPositions(const FVector& PreviousTilePosition);

	// Tail target positions – one for each tail segment.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snake")
	TArray<FVector> TailTargetPositions;

	// Overlap event for food pickup.
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult & SweepResult);


protected:
	// So that we can utilize gravity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "Used for falling and jumping."))
	float VelocityZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "If the snake is in air."))
	bool bInAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "Speed of the snake (cm / second)."))
	float Speed = 500.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "The forward rotation of the snake."))
	FRotator ForwardRotation;

	// Direction queue that gets set and poped each time the snake reaches a new tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ESnakeDirection> DirectionQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "How long the snake has moved since reaching the last tile."))
	float MovedTileDistance = 0.0f;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Updates the direction each time a new tile is reached
	UFUNCTION()
	void UpdateDirection();

	// Updates snake movment (called from Tick)
	UFUNCTION()
	void UpdateMovement(float DeltaTime);

	// Move the snake in the set distance
	UFUNCTION()
	void MoveSnake(float Distance);

	// Updates falling (called from Tick)
	UFUNCTION()
	void UpdateFalling(float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Blueprint callable Jump function
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Makes the snake jump."))
	void Jump();

	// Blueprint callable SetNextDirection function
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Add a direction onto a queue where the first in line direction gets set and poped."))
	void SetNextDirection(ESnakeDirection InDirection);
};
