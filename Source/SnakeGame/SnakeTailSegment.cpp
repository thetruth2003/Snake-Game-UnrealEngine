#include "SnakeTailSegment.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASnakeTailSegment::ASnakeTailSegment()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Load a default mesh from engine content (e.g., a Cube)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cube mesh not found!"));
	}

	// Optionally, adjust scale if necessary.
	MeshComponent->SetWorldScale3D(FVector(0.5f));  // Adjust the scale as needed.
}
