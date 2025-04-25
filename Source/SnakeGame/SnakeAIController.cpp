// SnakeAIController.cpp
#include "SnakeAIController.h"

#include <queue>

#include "SnakePawn.h"
#include "SnakeWorld.h"
#include "SnakeFood.h"
#include "Definitions.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASnakeAIController::ASnakeAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASnakeAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ASnakePawn* Snake = Cast<ASnakePawn>(GetPawn());
    if (!Snake) return;

    // Only recalc when we've actually moved into a new tile
    if (Snake->LastTilePosition.Equals(PrevTilePosition, 1e-3f))
        return;
    PrevTilePosition = Snake->LastTilePosition;

    // Find & snap the closest apple
    TArray<AActor*> Foods;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeFood::StaticClass(), Foods);
    if (Foods.Num() == 0) return;

    AActor* Closest = Foods[0];
    float Best = FVector::Dist(PrevTilePosition, Closest->GetActorLocation());
    for (AActor* F : Foods)
    {
        float D = FVector::Dist(PrevTilePosition, F->GetActorLocation());
        if (D < Best) { Best = D; Closest = F; }
    }
    FVector Goal = SnapToGrid(Closest->GetActorLocation());

    // Run BFS
    TArray<FVector> Path;
    if (!FindPath(PrevTilePosition, Goal, Path) || Path.Num() < 2)
        return;

    // Debug draw
    for (int32 i = 0; i < Path.Num(); ++i)
    {
        DrawDebugSphere(GetWorld(), Path[i], TileSize * 0.2f, 8, FColor::Yellow, false, 0.1f);
        if (i < Path.Num() - 1)
            DrawDebugLine(GetWorld(), Path[i], Path[i+1], FColor::Blue, false, 0.1f, 0, 5.f);
    }

    // Next step delta
    FVector Delta = Path[1] - PrevTilePosition;
    ESnakeDirection Dir = ESnakeDirection::None;
    if (FMath::Abs(Delta.X) > FMath::Abs(Delta.Y))
        Dir = (Delta.X > 0) ? ESnakeDirection::Up : ESnakeDirection::Down;
    else
        Dir = (Delta.Y > 0) ? ESnakeDirection::Right : ESnakeDirection::Left;

    // NO U-turn: only skip the *set* if it’s opposite, but do NOT abort the rest of the tick
    auto IsOpposite = [](ESnakeDirection A, ESnakeDirection B){
        return (A == ESnakeDirection::Up    && B == ESnakeDirection::Down)  ||
               (A == ESnakeDirection::Down  && B == ESnakeDirection::Up)    ||
               (A == ESnakeDirection::Left  && B == ESnakeDirection::Right) ||
               (A == ESnakeDirection::Right && B == ESnakeDirection::Left);
    };

    if (!IsOpposite(Snake->Direction, Dir))
    {
        // Queue & face
        if (Snake->Direction != Dir)
        {
            Snake->SetNextDirection(Dir);
            Snake->Direction = Dir;  // ← FORCE immediate movement on this tick

            FRotator NewRot;
            switch (Dir)
            {
                case ESnakeDirection::Up:    NewRot = {0,   0,   0}; break;
                case ESnakeDirection::Right: NewRot = {0,  90,   0}; break;
                case ESnakeDirection::Down:  NewRot = {0, 180,   0}; break;
                case ESnakeDirection::Left:  NewRot = {0, 270,   0}; break;
                default:                     NewRot = Snake->GetActorRotation(); break;
            }
            Snake->SetActorRotation(NewRot);
        }
    }
    else
    {
        UE_LOG(LogTemp, Verbose,
               TEXT("AI: skipping U-turn from %s to %s"),
               *UEnum::GetValueAsString(Snake->Direction),
               *UEnum::GetValueAsString(Dir));
    }
}

FVector ASnakeAIController::SnapToGrid(const FVector& WorldPos)
{
    // Delegate to our shared helper
    return ::SnapToGrid(WorldPos);
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

