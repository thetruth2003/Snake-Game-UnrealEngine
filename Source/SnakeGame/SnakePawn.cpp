// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakePawn.h"
#include "SnakeTailSegment.h"
#include "SnakeFood.h"

// Sets default values
ASnakePawn::ASnakePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	RootComponent = SceneComponent;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));

	CollisionComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASnakePawn::BeginPlay()
{
	Super::BeginPlay();
    
	// Initialize our last tile position.
	LastTilePosition = GetActorLocation();

	// Bind the OnOverlapBegin event on the collision component.
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASnakePawn::OnOverlapBegin);
	}
}

// Called every frame
void ASnakePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFalling(DeltaTime);

	UpdateMovement(DeltaTime);
}

// Called to bind functionality to input
void ASnakePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASnakePawn::MoveSnake(float Distance)
{
	// Get snake location in the world
	FVector Position = GetActorLocation();

	// Update moving the snake
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

	// Set snake location in the world
	SetActorLocation(Position);

	MovedTileDistance += Distance;
}

void ASnakePawn::UpdateMovement(float DeltaTime)
{
	float TotalMoveDistance = Speed * DeltaTime;
	float MoveDistance = TotalMoveDistance;

	while (MovedTileDistance + MoveDistance >= TileSize)
	{
		// Calculate the distance needed to finish the current tile.
		MoveDistance = TileSize - MovedTileDistance;

		// Store the head’s tile position before it updates.
		FVector PreviousTilePos = LastTilePosition;

		// In UpdateMovement(), when a full tile has been traversed:
		FVector PreviousHeadPos = LastTilePosition;  // Save the last tile position before moving.
		MoveSnake(MoveDistance);
		UpdateTailTargets(PreviousHeadPos);  // Update all tail target positions.
		LastTilePosition = GetActorLocation(); // Update the head's last tile position.

		// Update tail segments so that the first segment moves
		// to the previous tile position.
		UpdateTailPositions(PreviousTilePos);

		// Now update the last tile position to the new head position.
		LastTilePosition = GetActorLocation();

		// Update the direction queue as per your existing logic.
		UpdateDirection();

		TotalMoveDistance -= MoveDistance;
		MoveDistance = TotalMoveDistance;
		MovedTileDistance -= TileSize;
	}

	// Move any remaining distance.
	if (MoveDistance > 0.0f)
	{
		MoveSnake(MoveDistance);
	}
}

void ASnakePawn::UpdateFalling(float DeltaTime)
{
	// Get snake location in the world
	FVector Position = GetActorLocation();

	// Update speed
	VelocityZ -= 10.0f * DeltaTime;

	// Update falling
	Position.Z += VelocityZ;

	// Check floor collisions
	if (Position.Z <= 0.0f)
	{
		// Invert the position
		Position.Z = -Position.Z;

		// Invert and damp the velocity
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

	// Set snake location in the world
	SetActorLocation(Position);
}

void ASnakePawn::Jump()
{
	// Makes the snake jump (if it is not in the air)
	if (!bInAir)
	{
		VelocityZ = 2.5f;
	}
}

void ASnakePawn::UpdateDirection()
{
	if (DirectionQueue.IsEmpty())
	{
		return;
	}

	Direction = DirectionQueue[0];

	DirectionQueue.RemoveAt(0);



	// Rotate the snake
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

void ASnakePawn::SetNextDirection(ESnakeDirection InDirection)
{
	DirectionQueue.Add(InDirection);
}

void ASnakePawn::GrowTail()
{
	if (!GetWorld()) return;

	FActorSpawnParameters SpawnParams;
	// Spawn the tail segment at the current head location (or LastTilePosition).
	ASnakeTailSegment* NewSegment = GetWorld()->SpawnActor<ASnakeTailSegment>(
		ASnakeTailSegment::StaticClass(), LastTilePosition, FRotator::ZeroRotator, SpawnParams);
	if (NewSegment)
	{
		TailSegments.Add(NewSegment);
		// Also add a target position entry for the new tail.
		TailTargetPositions.Add(LastTilePosition);
		UE_LOG(LogTemp, Warning, TEXT("Tail grown. Total segments: %d"), TailSegments.Num());
	}
}

void ASnakePawn::UpdateTailTargets(const FVector& PreviousHeadPosition)
{
	if (TailSegments.Num() > 0)
	{
		// The first segment should target where the head was.
		TailTargetPositions[0] = PreviousHeadPosition;

		// Every subsequent tail segment should target the previous segment’s current location.
		for (int32 i = 1; i < TailSegments.Num(); i++)
		{
			TailTargetPositions[i] = TailSegments[i - 1]->GetActorLocation();
		}
	}
}


void ASnakePawn::UpdateTailPositions(const FVector& PreviousTilePosition)
{
	if (TailSegments.Num() > 0)
	{
		// The first tail segment moves to where the head just was.
		TailSegments[0]->SetActorLocation(PreviousTilePosition);

		// Each subsequent segment moves to the previous segment's old position.
		for (int32 i = 1; i < TailSegments.Num(); i++)
		{
			FVector PrevPos = TailSegments[i - 1]->GetActorLocation();
			TailSegments[i]->SetActorLocation(PrevPos);
		}
	}
}

void ASnakePawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlap detected with %s"), *OtherActor->GetName());
	}

	if (OtherActor && OtherActor->IsA(ASnakeFood::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Food overlap detected - calling GrowTail()"));
		GrowTail();
		OtherActor->Destroy();
	}
}

