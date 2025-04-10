#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeFood.generated.h"

UCLASS()
class SNAKEGAME_API ASnakeFood : public AActor
{
	GENERATED_BODY()
	
public:	
	ASnakeFood();

	// The food’s visual representation.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;
};
