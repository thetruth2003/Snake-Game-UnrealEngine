#include "SnakePawn.h"
#include "SnakeTailSegment.h"
#include "SnakeFood.h"
#include "SnakeWorld.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASnakePawn::ASnakePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned.
void ASnakePawn::BeginPlay()
{
	Super::BeginPlay();

	// Snap the snake to the nearest tile center.
	FVector SnappedLocation = SnapToGrid(GetActorLocation());
	SetActorLocation(SnappedLocation);
	LastTilePosition = SnappedLocation; // Ensure the movement system starts on grid.

	// Bind the overlap event on the collision component.
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASnakePawn::OnOverlapBegin);
	}
}

// Helper function that snaps a given location to the nearest tile center.
// You can use the first version if your floor/wall meshes are centered,
// or the alternate (commented) version if a half-tile offset is required.
FVector ASnakePawn::SnapToGrid(const FVector& InLocation) const
{
	// Without offset:
	/*
	float SnappedX = FMath::RoundToFloat(InLocation.X / TileSize) * TileSize;
	float SnappedY = FMath::RoundToFloat(InLocation.Y / TileSize) * TileSize;
	return FVector(SnappedX, SnappedY, InLocation.Z);
	*/
    
	 // Alternatively, with a half-tile offset (if needed):
	constexpr float HalfTile = TileSize / 2.0f;
	float SnappedX = FMath::RoundToFloat((InLocation.X - HalfTile) / TileSize) * TileSize + HalfTile;
	float SnappedY = FMath::RoundToFloat(InLocation.Y / TileSize) * TileSize;
	return FVector(SnappedX, SnappedY, InLocation.Z);
	
}

// Called every frame.
void ASnakePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update falling and movement.
	UpdateFalling(DeltaTime);
	UpdateMovement(DeltaTime);

	// Record the head's current position if it has moved a minimum distance.
	const float RecordDistance = 10.0f;
	FVector CurrentHeadPos = GetActorLocation();
	if (HeadPositionHistory.Num() == 0 ||
	    FVector::Dist(HeadPositionHistory.Last(), CurrentHeadPos) >= RecordDistance)
	{
		HeadPositionHistory.Add(CurrentHeadPos);
	}

	// Limit history length to avoid unbounded growth.
	int32 MaxHistoryLength = (TailSegments.Num() + 1) * TailHistorySpacing + 10;
	if (HeadPositionHistory.Num() > MaxHistoryLength)
	{
		HeadPositionHistory.RemoveAt(0, HeadPositionHistory.Num() - MaxHistoryLength);
	}

	// Update tail segments to follow the head history.
	const float SmoothSpeed = 10.0f; // Adjust as needed.
	for (int32 i = 0; i < TailSegments.Num(); i++)
	{
		int32 HistoryIndex = HeadPositionHistory.Num() - 1 - (i + 1) * TailHistorySpacing;
		HistoryIndex = FMath::Clamp(HistoryIndex, 0, HeadPositionHistory.Num() - 1);

		FVector TargetPos = HeadPositionHistory[HistoryIndex];
		FVector CurrentPos = TailSegments[i]->GetActorLocation();
		FVector NewPos = FMath::VInterpTo(CurrentPos, TargetPos, DeltaTime, SmoothSpeed);
		TailSegments[i]->SetActorLocation(NewPos);
	}
}

// Overlap event: Handles collision with food, tail, or walls.
void ASnakePawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSweep, const FHitResult & SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// Collision with Food: Grow tail, destroy the food, and request new food spawn.
	if (OtherActor->IsA(ASnakeFood::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Food overlap detected - calling GrowTail()"));
		GrowTail();
		OtherActor->Destroy();

		// Retrieve the SnakeWorld actor and spawn new food.
		ASnakeWorld* SnakeWorldActor = Cast<ASnakeWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass()));
		if (SnakeWorldActor)
		{
			SnakeWorldActor->SpawnFood();
		}
		return;
	}

	// Collision with Tail Segment: Trigger game over.
	if (OtherActor->IsA(ASnakeTailSegment::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Collision with tail detected! Game Over!"));
		GameOver();
		return;
	}

	// Collision with Walls: Trigger game over.
	if (OtherActor->ActorHasTag("Wall"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Collision with wall detected! Game Over!"));
		GameOver();
		return;
	}
}

// GameOver: Currently restarts the level.
void ASnakePawn::GameOver()
{
	UE_LOG(LogTemp, Warning, TEXT("Game Over triggered in GameOver() function."));

	// Restart the current level.
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();
		CurrentLevel.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevel));
	}
}

// Binds functionality to input.
void ASnakePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Moves the snake by the given distance.
void ASnakePawn::MoveSnake(float Distance)
{
	FVector Position = GetActorLocation();

	// Update movement based on current direction.
	switch (Direction)
	{
	case ESnakeDirection::Up:
		Position.X += Distance;
		break;
	case ESnakeDirection::Right:
		Position.Y += Distance;
		break;
	case ESnakeDirection::Down:
		Position.X -= Distance;
		break;
	case ESnakeDirection::Left:
		Position.Y -= Distance;
		break;
	}

	SetActorLocation(Position);
	MovedTileDistance += Distance;
}

// Updates snake movement based on elapsed time.
void ASnakePawn::UpdateMovement(float DeltaTime)
{
	// Calculate total distance to move this tick.
	float DistanceToTravel = Speed * DeltaTime;
	FVector CurrentPosition = GetActorLocation();

	// Move in continuous steps until the remaining movement is less than needed to complete a tile.
	while (DistanceToTravel > 0.0f)
	{
		float StepDistance = FMath::Min(DistanceToTravel, TileSize - MovedTileDistance);
		// Move in the current direction.
		CurrentPosition += GetDirectionVector() * StepDistance;

		// Update the accumulated distance.
		MovedTileDistance += StepDistance;
		DistanceToTravel -= StepDistance;

		// If a full tile has been traversed, update state.
		if (FMath::IsNearlyEqual(MovedTileDistance, TileSize) || MovedTileDistance > TileSize)
		{
			// Snap to grid (optional – helps avoid drift).
			LastTilePosition = CurrentPosition;
			MovedTileDistance = 0.0f;

			// Update tail targets based on the completed tile move.
			UpdateTailTargets(LastTilePosition);

			// Check and update the next queued direction.
			UpdateDirection();
		}
	}

	// Finally set the new position.
	SetActorLocation(CurrentPosition);
}

// Updates falling behavior and handles collision with the floor.
void ASnakePawn::UpdateFalling(float DeltaTime)
{
	FVector Position = GetActorLocation();
	VelocityZ -= 10.0f * DeltaTime;
	Position.Z += VelocityZ;

	if (Position.Z <= 0.0f)
	{
		Position.Z = -Position.Z;
		VelocityZ = -VelocityZ * 0.5f;

		if (FMath::Abs(VelocityZ) < 0.1f)
		{
			VelocityZ = 0.0f;
			Position.Z = 0.0f;
			bInAir = false;
		}
	}
	else
	{
		bInAir = true;
	}

	SetActorLocation(Position);
}

// Makes the snake jump if not already in the air.
void ASnakePawn::Jump()
{
	if (!bInAir)
	{
		VelocityZ = 2.5f;
	}
}

// Updates the snake's direction from the queue.
void ASnakePawn::UpdateDirection()
{
	if (DirectionQueue.IsEmpty())
	{
		return;
	}

	Direction = DirectionQueue[0];
	DirectionQueue.RemoveAt(0);

	// Rotate the snake according to the new direction.
	switch (Direction)
	{
	case ESnakeDirection::Up:
		ForwardRotation = FRotator(0.0f, 0.0f, 0.0f);
		break;
	case ESnakeDirection::Right:
		ForwardRotation = FRotator(0.0f, 90.0f, 0.0f);
		break;
	case ESnakeDirection::Down:
		ForwardRotation = FRotator(0.0f, 180.0f, 0.0f);
		break;
	case ESnakeDirection::Left:
		ForwardRotation = FRotator(0.0f, 270.0f, 0.0f);
		break;
	}
}

// Returns a unit vector based on the current direction.
FVector ASnakePawn::GetDirectionVector() const
{
	switch (Direction)
	{
	case ESnakeDirection::Up:
		return FVector(1.0f, 0.0f, 0.0f);
	case ESnakeDirection::Right:
		return FVector(0.0f, 1.0f, 0.0f);
	case ESnakeDirection::Down:
		return FVector(-1.0f, 0.0f, 0.0f);
	case ESnakeDirection::Left:
		return FVector(0.0f, -1.0f, 0.0f);
	default:
		return FVector::ZeroVector;
	}
}

// Adds a new direction to the movement queue.
void ASnakePawn::SetNextDirection(ESnakeDirection InDirection)
{
	DirectionQueue.Add(InDirection);
}

// Spawns a new tail segment when food is eaten.
void ASnakePawn::GrowTail()
{
	if (!GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	UClass* SpawnClass = TailSegmentClass.Get() ? TailSegmentClass.Get() : ASnakeTailSegment::StaticClass();

	ASnakeTailSegment* NewSegment = GetWorld()->SpawnActor<ASnakeTailSegment>(SpawnClass, LastTilePosition, FRotator::ZeroRotator, SpawnParams);
	if (NewSegment)
	{
		TailSegments.Add(NewSegment);
		// Also add a target position entry for the new tail.
		TailTargetPositions.Add(LastTilePosition);
		UE_LOG(LogTemp, Warning, TEXT("Tail grown. Total segments: %d"), TailSegments.Num());
	}
}

// Updates tail targets to create smooth following.
void ASnakePawn::UpdateTailTargets(const FVector& PreviousHeadPosition)
{
	if (TailSegments.Num() > 0)
	{
		TailTargetPositions[0] = PreviousHeadPosition;
		for (int32 i = 1; i < TailSegments.Num(); i++)
		{
			TailTargetPositions[i] = TailTargetPositions[i - 1];
		}
	}
}

// Updates tail positions after moving a full tile.
void ASnakePawn::UpdateTailPositions(const FVector& PreviousTilePosition)
{
	if (TailSegments.Num() > 0)
	{
		TArray<FVector> OldPositions;
		OldPositions.SetNum(TailSegments.Num());

		for (int32 i = 0; i < TailSegments.Num(); i++)
		{
			OldPositions[i] = TailSegments[i]->GetActorLocation();
		}

		TailSegments[0]->SetActorLocation(PreviousTilePosition);

		for (int32 i = 1; i < TailSegments.Num(); i++)
		{
			TailSegments[i]->SetActorLocation(OldPositions[i - 1]);
		}
	}
}
