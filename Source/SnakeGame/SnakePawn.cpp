#include "SnakePawn.h"
#include "SnakeTailSegment.h"
#include "SnakeFood.h"
#include "SnakeGameMode.h"
#include "SnakeWorld.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASnakePawn::ASnakePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create the collision component and attach it.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
		
	// Hard override any Blueprint modifications: force collision to query-only and overlap with all channels.
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned.
void ASnakePawn::BeginPlay()
{
	Super::BeginPlay();

	// Reapply collision settings to override any changes made in Blueprint.
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
		CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		CollisionComponent->SetGenerateOverlapEvents(true);
	}

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
FVector ASnakePawn::SnapToGrid(const FVector& InLocation)
{
	// Without offset:
	
	float SnappedX = FMath::RoundToFloat(InLocation.X / TileSize) * TileSize;
	float SnappedY = FMath::RoundToFloat(InLocation.Y / TileSize) * TileSize;
	return FVector(SnappedX, SnappedY, InLocation.Z);
	
    
	 /* // Alternatively, with a half-tile offset (if needed):
	constexpr float HalfTile = TileSize / 2.0f;
	float SnappedX = FMath::RoundToFloat((InLocation.X - HalfTile) / TileSize) * TileSize + HalfTile;
	float SnappedY = FMath::RoundToFloat((InLocation.Y - HalfTile) / TileSize) * TileSize + HalfTile;
	return FVector(SnappedX, SnappedY, InLocation.Z);
	*/
	
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
// In SnakePawn.cpp

void ASnakePawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// Collision with Food: Grow tail, destroy the food, and request new food spawn.
	if (OtherActor->IsA(ASnakeFood::StaticClass()))
	{
		GrowTail();
		OtherActor->Destroy();

		// Notify the GameMode instead of directly spawning
		ASnakeGameMode* GM = Cast<ASnakeGameMode>(
			UGameplayStatics::GetGameMode(GetWorld())
		);
		if (GM)
		{
			GM->NotifyAppleEaten();
		}
		return;
	}

	// Collision with Tail Segment: Check the collision flag.
	if (OtherActor->IsA(ASnakeTailSegment::StaticClass()))
	{
		ASnakeTailSegment* TailSeg = Cast<ASnakeTailSegment>(OtherActor);
		// Only trigger game over if this tail segment is fully enabled for collision.
		if (TailSeg && TailSeg->bCanCollide)
		{
			UE_LOG(LogTemp, Warning, TEXT("Collision with tail detected! Game Over!"));
			GameOver();
			return;
		}
		else
		{
			// Ignore collision if the tail segment is still in its 'cooldown' phase.
			return;
		}
	}

	// Collision with Walls: Check if the actor or component has the "Wall" tag.
	if (OtherActor->ActorHasTag("Wall") || (OtherComp && OtherComp->ComponentHasTag("Wall")))
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

	// Get a reference to your custom game mode.
	ASnakeGameMode* GameMode = Cast<ASnakeGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		// Transition to the Game Over state.
		GameMode->SetGameState(EGameState::Outro);
	}

	/* // Restart the current level.
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();
		CurrentLevel.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevel));
	}
	*/
}

void ASnakePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ASnakePawn::HandlePauseToggle);
	PlayerInputComponent->BindAction("TriggerGameOver", IE_Pressed, this, &ASnakePawn::GameOver);

}

void ASnakePawn::HandlePauseToggle()
{
	ASnakeGameMode* GM = Cast<ASnakeGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	if (GM->CurrentState == EGameState::Game)
		GM->SetGameState(EGameState::Pause);
	else if (GM->CurrentState == EGameState::Pause)
		GM->SetGameState(EGameState::Game);
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
		// Ensure collision is disabled and the flag is false.
		NewSegment->MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NewSegment->bCanCollide = false;

		// Schedule re-enabling collision after a delay (0.3 seconds in this example).
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindLambda([NewSegment]()
		{
			if (NewSegment)
			{
				// Re-enable collision to QueryOnly.
				NewSegment->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				NewSegment->bCanCollide = true;
			}
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 0.3f, false);

		TailSegments.Add(NewSegment);
		// Also add a target position entry for the new tail segment.
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
