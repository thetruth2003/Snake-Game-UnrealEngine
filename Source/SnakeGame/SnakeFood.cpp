#include "SnakeFood.h"
#include "Components/StaticMeshComponent.h"

ASnakeFood::ASnakeFood()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	
}
