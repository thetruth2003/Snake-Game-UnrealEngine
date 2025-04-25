// SnakeAIController.cpp
#include "SnakeAIController.h"

#include <queue>

#include "SnakePawn.h"
#include "SnakeWorld.h"
#include "SnakeFood.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASnakeAIController::ASnakeAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

FVector ASnakeAIController::SnapToGrid(const FVector& WorldPos) const
{
    float X = FMath::RoundToFloat(WorldPos.X / TileSize) * TileSize;
    float Y = FMath::RoundToFloat(WorldPos.Y / TileSize) * TileSize;
    return FVector(X, Y, WorldPos.Z);
}


void ASnakeAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ASnakePawn* Snake = Cast<ASnakePawn>(GetPawn());
    if (!Snake) return;

    // Only recalc when we've actually moved into a new tile
    FVector CurrentTile = Snake->LastTilePosition;
    if (CurrentTile.Equals(PrevTilePosition, 1e-3f))
        return;
    PrevTilePosition = CurrentTile;

    // Find & snap the closest apple
    TArray<AActor*> Foods;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeFood::StaticClass(), Foods);
    if (Foods.Num() == 0) return;
    AActor* Closest = Foods[0];
    float Best = FVector::Dist(CurrentTile, Closest->GetActorLocation());
    for (AActor* F : Foods)
    {
        float D = FVector::Dist(CurrentTile, F->GetActorLocation());
        if (D < Best) { Best = D; Closest = F; }
    }
    FVector Goal = SnapToGrid(Closest->GetActorLocation());

    // Run your BFS pathfinder
    TArray<FVector> Path;
    if (!FindPath(CurrentTile, Goal, Path) || Path.Num() < 2)
        return;

    // Draw debug path (yellow spheres + blue lines)
    for (int32 i = 0; i < Path.Num(); ++i)
    {
        DrawDebugSphere(GetWorld(), Path[i], TileSize * 0.2f, 8, FColor::Yellow,
                        false, 0.1f);
        if (i < Path.Num() - 1)
            DrawDebugLine(GetWorld(), Path[i], Path[i+1],
                          FColor::Blue, false, 0.1f, 0, 5.0f);
    }

    // Determine the very next step
    FVector Delta = Path[1] - CurrentTile;
    ESnakeDirection Dir = ESnakeDirection::None;
    if (FMath::Abs(Delta.X) > FMath::Abs(Delta.Y))
        Dir = (Delta.X > 0) ? ESnakeDirection::Up : ESnakeDirection::Down;
    else
        Dir = (Delta.Y > 0) ? ESnakeDirection::Right : ESnakeDirection::Left;

    // ─── NO U-TURNS: skip if Dir is exact opposite of current ───
    auto IsOpposite = [](ESnakeDirection A, ESnakeDirection B){
        return (A == ESnakeDirection::Up    && B == ESnakeDirection::Down)  ||
               (A == ESnakeDirection::Down  && B == ESnakeDirection::Up)    ||
               (A == ESnakeDirection::Left  && B == ESnakeDirection::Right) ||
               (A == ESnakeDirection::Right && B == ESnakeDirection::Left);
    };
    if (Snake->Direction != ESnakeDirection::None && IsOpposite(Snake->Direction, Dir))
    {
        UE_LOG(LogTemp, Verbose,
               TEXT("AI: would U-turn from %s to %s — skipping"),
               *UEnum::GetValueAsString(Snake->Direction),
               *UEnum::GetValueAsString(Dir));
        return;
    }

    // Queue & face that direction
    if (Snake->Direction != Dir)
    {
        Snake->SetNextDirection(Dir);

        // Face immediately
        FRotator NewRot;
        switch (Dir)
        {
            case ESnakeDirection::Up:    NewRot = FRotator(0,   0, 0); break;
            case ESnakeDirection::Right: NewRot = FRotator(0,  90, 0); break;
            case ESnakeDirection::Down:  NewRot = FRotator(0, 180, 0); break;
            case ESnakeDirection::Left:  NewRot = FRotator(0, 270, 0); break;
            default:                     NewRot = Snake->GetActorRotation(); break;
        }
        Snake->SetActorRotation(NewRot);
    }
}

bool ASnakeAIController::FindPath(
    const FVector& Start,
    const FVector& Goal,
    TArray<FVector>& OutPath
) const
{
    // Simple BFS on a grid. Assumes ASnakeWorld::FloorTileLocations is your walkable set.
    ASnakeWorld* World = Cast<ASnakeWorld>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())
    );
    if (!World) return false;

    // Build a quick lookup of valid tiles
    TSet<FVector> Walkable(World->FloorTileLocations);

    auto Snap = [&](const FVector& V){
        return FVector(
            FMath::RoundToFloat(V.X / TileSize) * TileSize,
            FMath::RoundToFloat(V.Y / TileSize) * TileSize,
            V.Z
        );
    };

    FVector S = Snap(Start), G = Snap(Goal);
    if (!Walkable.Contains(G)) return false;

    std::queue<FVector> Q;
    Q.push(S);

    TMap<FVector, FVector> CameFrom;
    CameFrom.Add(S, S);

    const TArray<FVector> Directions = {
        FVector(TileSize, 0, 0),
        FVector(-TileSize, 0, 0),
        FVector(0, TileSize, 0),
        FVector(0, -TileSize, 0)
    };

    while (!Q.empty())
    {
        FVector Curr = Q.front(); Q.pop();
        if (Curr == G) break;

        for (auto& Dir : Directions)
        {
            FVector Next = Curr + Dir;
            if (!Walkable.Contains(Next) || CameFrom.Contains(Next))
                continue;
            CameFrom.Add(Next, Curr);
            Q.push(Next);
        }
    }

    // Reconstruct path
    if (!CameFrom.Contains(G))
        return false;

    TArray<FVector> ReversePath;
    for (FVector At = G; At != S; At = CameFrom[At])
        ReversePath.Add(At);
    ReversePath.Add(S);

    // Flip it
    for (int32 i = ReversePath.Num() - 1; i >= 0; --i)
        OutPath.Add(ReversePath[i]);

    return true;
}

