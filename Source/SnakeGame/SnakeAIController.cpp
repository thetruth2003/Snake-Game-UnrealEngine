// SnakeAIController.cpp
#include "SnakeAIController.h"
#include "SnakePawn.h"
#include "SnakeWorld.h"
#include "SnakeFood.h"
#include "Kismet/GameplayStatics.h"

ASnakeAIController::ASnakeAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

FVector ASnakeAIController::SnapToGrid(const FVector& WorldPos) const
{
    // Round X/Y to nearest tile, preserve Z
    const float X = FMath::RoundToFloat(WorldPos.X / TileSize) * TileSize;
    const float Y = FMath::RoundToFloat(WorldPos.Y / TileSize) * TileSize;
    return FVector(X, Y, WorldPos.Z);
}

bool ASnakeAIController::FindPath(const FVector& Start, const FVector& Goal, TArray<FVector>& OutPath) const
{
    // Get our SnakeWorld (holds floor tile list & actor origin)
    ASnakeWorld* SW = Cast<ASnakeWorld>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())
    );
    if (!SW) return false;

    // Origin of grid
    FVector Origin = SW->GetActorLocation();

    // Build valid grid points from your FloorTileLocations
    TSet<FIntPoint> Valid;
    for (const FVector& LocalOffset : SW->GetFloorTileLocations())
    {
        int32 ix = FMath::RoundToInt(LocalOffset.X / TileSize);
        int32 iy = FMath::RoundToInt(LocalOffset.Y / TileSize);
        Valid.Add(FIntPoint(ix, iy));
    }

    // Convert Start/Goal into grid points RELATIVE to Origin
    FVector LocalStart = Start - Origin;
    FVector LocalGoal  = Goal  - Origin;

    FIntPoint StartPt(
        FMath::RoundToInt(LocalStart.X / TileSize),
        FMath::RoundToInt(LocalStart.Y / TileSize)
    );
    FIntPoint GoalPt(
        FMath::RoundToInt(LocalGoal.X / TileSize),
        FMath::RoundToInt(LocalGoal.Y / TileSize)
    );

    if (!Valid.Contains(StartPt) || !Valid.Contains(GoalPt))
        return false;

    // BFS
    TQueue<FIntPoint> Queue;
    TMap<FIntPoint, FIntPoint> Parent;
    Queue.Enqueue(StartPt);
    Parent.Add(StartPt, StartPt);

    static const FIntPoint Dirs[4] = {
        { 1,  0}, {-1,  0},
        { 0,  1}, { 0, -1}
    };

    bool bFound = false;
    while (!Queue.IsEmpty())
    {
        FIntPoint Curr;
        Queue.Dequeue(Curr);
        if (Curr == GoalPt) { bFound = true; break; }

        for (auto& D : Dirs)
        {
            FIntPoint Next = Curr + D;
            if (!Parent.Contains(Next) && Valid.Contains(Next))
            {
                Parent.Add(Next, Curr);
                Queue.Enqueue(Next);
            }
        }
    }

    if (!bFound) return false;

    // Reconstruct, then reverse into world positions
    TArray<FIntPoint> Rev;
    for (FIntPoint At = GoalPt; ; At = Parent[At])
    {
        Rev.Add(At);
        if (At == StartPt) break;
    }

    OutPath.Reset();
    for (int32 i = Rev.Num() - 1; i >= 0; --i)
    {
        const FIntPoint& P = Rev[i];
        FVector WorldTile = Origin + FVector(P.X * TileSize, P.Y * TileSize, 0.0f);
        OutPath.Add(WorldTile);
    }

    return true;
}

void ASnakeAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ASnakePawn* Snake = Cast<ASnakePawn>(GetPawn());
    if (!Snake) return;

    // 1) Snap our start to the grid
    FVector Start = SnapToGrid(Snake->GetActorLocation());

    // 2) Find nearest apple and snap it
    TArray<AActor*> Foods;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeFood::StaticClass(), Foods);
    if (Foods.Num() == 0) return;

    AActor* Closest = Foods[0];
    float BestDist = FVector::Dist(Start, Closest->GetActorLocation());
    for (AActor* F : Foods)
    {
        float D = FVector::Dist(Start, F->GetActorLocation());
        if (D < BestDist) { BestDist = D; Closest = F; }
    }
    FVector Goal = SnapToGrid(Closest->GetActorLocation());

    // 3) Run BFS
    TArray<FVector> Path;
    if (!FindPath(Start, Goal, Path) || Path.Num() < 2)
        return;  // no valid path or only at goal

    // 4) Pick the very next tile
    FVector Next = Path[1] - Start;

    // 5) Translate into a direction
    ESnakeDirection Dir = ESnakeDirection::None;
    if (FMath::Abs(Next.X) > FMath::Abs(Next.Y))
        Dir = (Next.X > 0) ? ESnakeDirection::Up : ESnakeDirection::Down;
    else
        Dir = (Next.Y > 0) ? ESnakeDirection::Right : ESnakeDirection::Left;

    // 6) Queue it
    if (Snake->Direction != Dir)
        Snake->SetNextDirection(Dir);
}
