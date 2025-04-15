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

	// Hard override collision properties:
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	// Adjust mesh scale as needed.
	MeshComponent->SetWorldScale3D(FVector(0.5f));

}
