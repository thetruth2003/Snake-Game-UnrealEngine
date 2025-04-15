#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeTailSegment.generated.h"

UCLASS()
class SNAKEGAME_API ASnakeTailSegment : public AActor
{
	GENERATED_BODY()
	
public:	
	ASnakeTailSegment();

	// A simple mesh to represent the tail segment.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// Flag to disable collision until the tail segment has had time to move away from the head.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	bool bCanCollide = false;
};
