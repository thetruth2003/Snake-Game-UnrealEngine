// SnakeGameMode.cpp

#include "SnakeGameMode.h"
#include "Engine/LocalPlayer.h" 
#include "Definitions.h"              // for SnapToGrid
#include "SnakeAIController.h"
#include "SnakeWorld.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h" 

ASnakeGameMode::ASnakeGameMode()
    : CurrentWidget(nullptr)
{
    CurrentState = EGameState::MainMenu;
}

void ASnakeGameMode::BeginPlay()
{
    Super::BeginPlay();
    SetGameState(CurrentState);
}

void ASnakeGameMode::SetGameType(EGameType NewType)
{
    CurrentGameType = NewType;
    UE_LOG(LogTemp, Log, TEXT("GameType set to %s"),
           *UEnum::GetValueAsString(NewType));

    UWorld* W = GetWorld();
    if (!W) return;

    // ── 1) Spawn second human player for Coop or PvP ──
    if ((NewType == EGameType::Coop || NewType == EGameType::PvP)
        && GetGameInstance()->GetNumLocalPlayers() < 2)
    {
        UGameplayStatics::CreatePlayer(W, 1, true);
        UE_LOG(LogTemp, Log, TEXT("Created second local player (ID 1)"));
    }

    // ── 2) Spawn AI snake for PvAI or CoopAI ──
    if (NewType == EGameType::PvAI || NewType == EGameType::CoopAI)
    {
        if (!IsValid(SpawnedAISnake))
        {
            // Find PlayerStart2 or fallback
            FTransform SpawnT;
            APlayerStart* P2 = nullptr;
            TArray<AActor*> Starts;
            UGameplayStatics::GetAllActorsOfClass(W, APlayerStart::StaticClass(), Starts);
            for (AActor* A : Starts)
            {
                if (A->ActorHasTag(TEXT("PlayerStart2")))
                {
                    P2 = Cast<APlayerStart>(A);
                    break;
                }
            }

            if (P2)
            {
                SpawnT = P2->GetActorTransform();
            }
            else
            {
                FVector RawFallback(400.f, 1000.f, 0.f);
                FVector Snapped = SnapToGrid(RawFallback);
                SpawnT.SetLocation(Snapped);
                UE_LOG(LogTemp, Warning,
                       TEXT("PlayerStart2 not found, using fallback at %s."),
                       *Snapped.ToString());
            }

            auto& ChosenBP = AISnakePawnBP ? AISnakePawnBP : Player2PawnBP;
            if (ChosenBP)
            {
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                ASnakePawn* NewAI = W->SpawnActor<ASnakePawn>(ChosenBP, SpawnT, Params);
                if (NewAI)
                {
                    SpawnedAISnake = NewAI;
                    ASnakeAIController* AICon = W->SpawnActor<ASnakeAIController>(
                        ASnakeAIController::StaticClass());
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

    // ── 3) Go live ──
    SetGameState(EGameState::Game);
}

void ASnakeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 Id = NewPlayer->GetLocalPlayer()->GetControllerId();
    if (Id == 1 && (CurrentGameType == EGameType::Coop || CurrentGameType == EGameType::PvP))
    {
        // Find PlayerStart2 or fallback
        APlayerStart* TargetStart = nullptr;
        TArray<AActor*> Starts;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Starts);
        for (AActor* Actor : Starts)
        {
            if (Actor->ActorHasTag(TEXT("PlayerStart2")))
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
            FVector RawFallback(400.f, 1000.f, 0.f);
            FVector Snapped = SnapToGrid(RawFallback);
            SpawnTransform.SetLocation(Snapped);
            UE_LOG(LogTemp, Warning,
                   TEXT("PlayerStart2 not found (PostLogin), using fallback at %s."),
                   *Snapped.ToString());
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        if (Player2PawnBP)
        {
            ASnakePawn* Pawn = GetWorld()->SpawnActor<ASnakePawn>(
                Player2PawnBP, SpawnTransform, Params);
            if (Pawn)
            {
                NewPlayer->Possess(Pawn);
                UE_LOG(LogTemp, Log,
                       TEXT("Spawned P2 at %s"),
                       *SpawnTransform.GetLocation().ToString());
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

AActor* ASnakeGameMode::ChoosePlayerStart_Implementation(AController* Controller)
{
    int32 ControllerId = 0;

    // 1) Cast to APlayerController
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        // 2) Get the ULocalPlayer
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            // 3) Pull the controller index
            ControllerId = LP->GetControllerId();
        }
    }

    // Decide tag based on ID
    FName DesiredTag = (ControllerId == 1) ? TEXT("PlayerStart2") : TEXT("PlayerStart1");

    // Find the properly tagged PlayerStart
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->ActorHasTag(DesiredTag))
        {
            UE_LOG(LogTemp, Log, TEXT("Spawning Controller %d at %s"), 
                   ControllerId, *DesiredTag.ToString());
            return *It;
        }
    }

    // Fallback
    return Super::ChoosePlayerStart_Implementation(Controller);
}