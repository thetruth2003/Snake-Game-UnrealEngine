#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "InputMappingContext.h" 
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

	// Update tail positions based on the snake movement.
	void UpdateTailTargets(const FVector& PreviousHeadPosition);

	// Called after a full tile movement to update positions of each tail segment.
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void UpdateTailPositions(const FVector& PreviousTilePosition);

	// Tail target positions – one for each tail segment.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snake")
	TArray<FVector> TailTargetPositions;

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	void HandlePauseToggle();
	
	// Binds functionality to input.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Blueprint callable jump function.
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Makes the snake jump."))
	void Jump();

	// Blueprint callable function to add a direction to the queue.
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Add a direction onto a queue where the first in line direction gets set and popped."))
	void SetNextDirection(ESnakeDirection InDirection);

	// Overlap event for collision handling.
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                    bool bFromSweep, const FHitResult & SweepResult);

	// Game over function – called when a fatal collision occurs.
	UFUNCTION(BlueprintCallable, Category = "Game")
	void GameOver();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Snake")
	TSubclassOf<ASnakeTailSegment> TailSegmentClass;

protected:
	// For gravity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "Used for falling and jumping."))
	float VelocityZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "If the snake is in air."))
	bool bInAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "Speed of the snake (cm / second)."))
	float Speed = 500.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "The forward rotation of the snake."))
	FRotator ForwardRotation;

	// Direction queue that gets set and popped each time the snake reaches a new tile.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ESnakeDirection> DirectionQueue;

	// Tracks the distance moved within the current tile.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ToolTip = "How long the snake has moved since reaching the last tile."))
	float MovedTileDistance = 0.0f;

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Updates the direction each time a new tile is reached.
	UFUNCTION()
	void UpdateDirection();

	FVector GetDirectionVector() const;

	// Updates snake movement (called each Tick).
	UFUNCTION()
	void UpdateMovement(float DeltaTime);

	// Moves the snake by a specified distance.
	UFUNCTION()
	void MoveSnake(float Distance);

	// Updates falling (called each Tick).
	UFUNCTION()
	void UpdateFalling(float DeltaTime);

private:
	// Records recent head positions.
	TArray<FVector> HeadPositionHistory;

	// Number of history entries between each tail segment.
	UPROPERTY(EditAnywhere, Category = "Snake|Tail")
	int32 TailHistorySpacing = 5;

	// *** NEW: Helper function to snap a position to the grid ***
	static FVector SnapToGrid(const FVector& InLocation);
};
