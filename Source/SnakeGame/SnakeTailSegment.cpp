#include "SnakeTailSegment.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASnakeTailSegment::ASnakeTailSegment()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the mesh component and set as the root.
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Load a default cube mesh.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cube mesh not found!"));
	}

	// Adjust the scale if necessary.
	MeshComponent->SetWorldScale3D(FVector(0.5f));

	// Set collision enabled so that overlaps are generated.
	// Using QueryOnly ensures that the mesh doesn't block movement but still fires overlap events.
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Set the collision object type to WorldDynamic (or choose a channel that makes sense for your game).
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	// Configure the collision response to overlap with all channels.
	// This way, when the snake's head (or its sphere collider) overlaps the tail, an event is generated.
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}
