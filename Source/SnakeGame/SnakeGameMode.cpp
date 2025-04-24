// SnakeGameMode.cpp
#include "SnakeGameMode.h"

#include "SnakeAIController.h"
#include "SnakeWorld.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerStart.h"

ASnakeGameMode::ASnakeGameMode()
    : CurrentWidget(nullptr)
{
    CurrentState = EGameState::MainMenu;
}

void ASnakeGameMode::BeginPlay()
{
    Super::BeginPlay();

    SetGameState(CurrentState);
    if (MainMenuWidgetClass)
    {
        CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
        CurrentWidget->AddToViewport();
    }
}

void ASnakeGameMode::SetGameType(EGameType NewType)
{
    CurrentGameType = NewType;
    UE_LOG(LogTemp, Log, TEXT("GameType set to %s"),
           *UEnum::GetValueAsString(NewType));

    UWorld* W = GetWorld();
    if (!W) return;

    // 1) If Coop or PvP, spawn second human player:
    if ((NewType == EGameType::Coop || NewType == EGameType::PvP)
        && GetGameInstance()->GetNumLocalPlayers() < 2)
    {
        UGameplayStatics::CreatePlayer(W, 1, true);
    }

    // 2) If PvAI or CoopAI, spawn exactly one AI snake:
    if (NewType == EGameType::PvAI || NewType == EGameType::CoopAI)
    {
        if (!SpawnedAISnake)
        {
            // find PlayerStart2 or fallback:
            FTransform SpawnT;
            APlayerStart* P2 = nullptr;
            TArray<AActor*> Starts;
            UGameplayStatics::GetAllActorsOfClass(W, APlayerStart::StaticClass(), Starts);
            for (AActor* A : Starts)
            {
                if (A->ActorHasTag(FName("PlayerStart2")))
                {
                    P2 = Cast<APlayerStart>(A);
                    break;
                }
            }
            if (P2) SpawnT = P2->GetActorTransform();
            else
            {
                SpawnT.SetLocation(FVector(400,1000,0));
                UE_LOG(LogTemp, Warning,
                       TEXT("PlayerStart2 not found, using fallback location."));
            }

            // choose which BP to spawn: AI‐specific or reuse P2
            auto& ChosenBP = AISnakePawnBP ? AISnakePawnBP : Player2PawnBP;
            if (ChosenBP)
            {
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                // spawn the pawn
                ASnakePawn* NewAI = W->SpawnActor<ASnakePawn>(ChosenBP, SpawnT, Params);
                if (NewAI)
                {
                    SpawnedAISnake = NewAI;

                    // spawn & possess with our AI Controller
                    ASnakeAIController* AICon =
                        W->SpawnActor<ASnakeAIController>(ASnakeAIController::StaticClass());
                    if (AICon)
                    {
                        AICon->Possess(NewAI);
                        UE_LOG(LogTemp, Log,
                               TEXT("Spawned & possessed AI snake with %s"),
                               *ChosenBP->GetName());
                    }
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AI snake already spawned; skipping."));
        }
    }

    // 3) Finally, go live
    SetGameState(EGameState::Game);
}

void ASnakeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 Id = NewPlayer->GetLocalPlayer()->GetControllerId();
    if (Id == 1 && (CurrentGameType == EGameType::Coop || CurrentGameType == EGameType::PvP))
    {
        // Try to find the PlayerStart with tag "PlayerStart2"
        APlayerStart* TargetStart = nullptr;
        TArray<AActor*> FoundStarts;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundStarts);
        for (AActor* Actor : FoundStarts)
        {
            if (Actor->ActorHasTag(FName("PlayerStart2")))
            {
                TargetStart = Cast<APlayerStart>(Actor);
                break;
            }
        }

        FTransform SpawnTransform;
        if (TargetStart)
        {
            SpawnTransform = TargetStart->GetActorTransform();
        }
        else
        {
            // Fallback to your old location
            SpawnTransform = FTransform(FRotator::ZeroRotator, FVector(400,1000,0));
            UE_LOG(LogTemp, Warning, TEXT("PlayerStart2 not found, using fallback location."));
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        if (Player2PawnBP)
        {
            ASnakePawn* Pawn = GetWorld()->SpawnActor<ASnakePawn>(
                Player2PawnBP, SpawnTransform, Params);
            if (Pawn)
            {
                NewPlayer->Possess(Pawn);
                UE_LOG(LogTemp, Log, TEXT("Spawned P2 at %s"), *SpawnTransform.GetLocation().ToString());
            }
        }
    }
}

void ASnakeGameMode::NotifyAppleEaten()
{
    ApplesEaten++;

    ASnakeWorld* World = Cast<ASnakeWorld>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass()));
    if (!World) return;

    if (ApplesEaten < ApplesToFinish)
    {
        World->SpawnFood();
    }
    else
    {
        SetGameState(EGameState::Pause);
        int32 Next = World->LevelIndex + 1;
        if (!World->DoesLevelExist(Next))
        {
            SetGameState(EGameState::Outro);
            return;
        }
        World->LevelIndex = Next;
        World->LoadLevelFromText();
        World->SpawnFood();
        ApplesEaten = 0;
        SetGameState(EGameState::Game);
    }
}

void ASnakeGameMode::SetGameState(EGameState NewState)
{
    if (CurrentWidget)    { CurrentWidget->RemoveFromParent(); CurrentWidget = nullptr; }
    if (PauseWidget)      { PauseWidget->RemoveFromParent(); PauseWidget = nullptr; }
    CurrentState = NewState;

    switch (CurrentState)
    {
        case EGameState::MainMenu:
            UGameplayStatics::SetGamePaused(GetWorld(), true);
            if (MainMenuWidgetClass)
                CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
            break;

        case EGameState::Game:
            UGameplayStatics::SetGamePaused(GetWorld(), false);
            break;

        case EGameState::Pause:
            UGameplayStatics::SetGamePaused(GetWorld(), true);
            if (PauseMenuWidgetClass)
                PauseWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuWidgetClass);
            break;

        case EGameState::Outro:
            UGameplayStatics::SetGamePaused(GetWorld(), true);
            if (GameOverWidgetClass)
                CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
            break;
    }

    if (CurrentWidget) CurrentWidget->AddToViewport();
    if (PauseWidget)   PauseWidget->AddToViewport();
}
