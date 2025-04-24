// SnakeAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Definitions.h"       // for TileSize & ESnakeDirection
#include "SnakeAIController.generated.h"

UCLASS()
class SNAKEGAME_API ASnakeAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASnakeAIController();
	virtual void Tick(float DeltaTime) override;

private:
	/** 
	 * BFS on the floor-tile grid from Start→Goal.
	 * OutPath will be world locations for each tile center.
	 */
	bool FindPath(const FVector& Start, const FVector& Goal, TArray<FVector>& OutPath) const;

	/** Snaps any world position to the nearest Tile-center. */
	FVector SnapToGrid(const FVector& WorldPos) const;
};
