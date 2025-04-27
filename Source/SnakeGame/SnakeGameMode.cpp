#include "SnakeGameMode.h"
#include "MyUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "SnakeWorld.h"
#include "SnakeAIController.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

ASnakeGameMode::ASnakeGameMode()
    : CurrentWidget(nullptr)
    , PauseWidget(nullptr)
    , InGameWidget(nullptr)
    , SpawnedAISnake(nullptr)
    , ApplesToFinish(5)
    , ApplesEaten(0)
    , Score(0)
    , ApplesEatenP1(0)
    , ApplesEatenP2(0)
    , CurrentGameType(EGameType::SinglePlayer)
    , CurrentState(EGameState::MainMenu)
{
}

void ASnakeGameMode::BeginPlay()
{
    Super::BeginPlay();
    SetGameState(CurrentState);
}

void ASnakeGameMode::SetGameType(EGameType NewType)
{
    // —— Cleanup extra local player if switching out of any 2-player human mode ——
    if (GetGameInstance()->GetNumLocalPlayers() > 1
        && NewType != EGameType::Coop
        && NewType != EGameType::PvP
        && NewType != EGameType::CoopAI)
    {
        if (ULocalPlayer* Extra = GetGameInstance()->GetLocalPlayers()[1])
        {
            GetGameInstance()->RemoveLocalPlayer(Extra);
            UE_LOG(LogTemp, Log,
                   TEXT("Removed extra local player when switching to %s"),
                   *UEnum::GetValueAsString(NewType));
        }
    }

    // —— Cleanup AI snake if leaving any AI mode ——
    if (SpawnedAISnake
        && NewType != EGameType::PvAI
        && NewType != EGameType::CoopAI)
    {
        if (AController* AICon = SpawnedAISnake->GetController())
        {
            AICon->Destroy();
        }
        SpawnedAISnake->Destroy();
        SpawnedAISnake = nullptr;
        UE_LOG(LogTemp, Log,
               TEXT("Destroyed AI snake when switching to %s"),
               *UEnum::GetValueAsString(NewType));
    }

    // —— Core spawn logic ——  
    CurrentGameType = NewType;
    UE_LOG(LogTemp, Log, TEXT("GameType set to %s"),
           *UEnum::GetValueAsString(NewType));

    UWorld* W = GetWorld();
    if (!W) return;

    // 1) Spawn second human player for Coop or PvP
    if ((NewType == EGameType::Coop || NewType == EGameType::PvP)
        && GetGameInstance()->GetNumLocalPlayers() < 2)
    {
        UGameplayStatics::CreatePlayer(W, 1, true);
        UE_LOG(LogTemp, Log, TEXT("Created second local player (ID 1)"));
    }

    // 2) Spawn AI snake for PvAI or CoopAI
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

    // 3) Go live
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

void ASnakeGameMode::NotifyAppleEaten(int32 ControllerId)
{
    if (CurrentGameType == EGameType::PvP || CurrentGameType == EGameType::PvAI)
    {
        if (ControllerId == 0) ++ApplesEatenP1;
        else                   ++ApplesEatenP2;
    }
    else
    {
        ++ApplesEaten;
    }

    ++Score;

    if (CurrentState == EGameState::Game && InGameWidget)
    {
        if (CurrentGameType == EGameType::PvP || CurrentGameType == EGameType::PvAI)
        {
            InGameWidget->SetPlayerScores(ApplesEatenP1, ApplesEatenP2);
        }
        else
        {
            InGameWidget->SetScore(Score);
        }
    }

    ASnakeWorld* World = Cast<ASnakeWorld>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())
    );
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
    if (CurrentWidget) { CurrentWidget->RemoveFromParent(); CurrentWidget = nullptr; }
    if (PauseWidget)   { PauseWidget->RemoveFromParent();   PauseWidget   = nullptr; }

    CurrentState = NewState;
    switch (CurrentState)
    {
    case EGameState::MainMenu:
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        if (MainMenuWidgetClass)
        {
            CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
            if (CurrentWidget) CurrentWidget->AddToViewport();
        }
        break;

    case EGameState::Game:
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        if (!InGameWidget && InGameWidgetClass)
        {
            InGameWidget = CreateWidget<UMyUserWidget>(GetWorld(), InGameWidgetClass);
            if (InGameWidget)
            {
                InGameWidget->AddToViewport();
                // Show/hide score fields for PvP vs single
                if (CurrentGameType == EGameType::PvP || CurrentGameType == EGameType::PvAI)
                {
                    InGameWidget->ScoreText  ->SetVisibility(ESlateVisibility::Collapsed);
                    InGameWidget->ScoreP1Text->SetVisibility(ESlateVisibility::Visible);
                    InGameWidget->ScoreP2Text->SetVisibility(ESlateVisibility::Visible);
                    InGameWidget->SetPlayerScores(ApplesEatenP1, ApplesEatenP2);
                }
                else
                {
                    InGameWidget->ScoreText  ->SetVisibility(ESlateVisibility::Visible);
                    InGameWidget->ScoreP1Text->SetVisibility(ESlateVisibility::Collapsed);
                    InGameWidget->ScoreP2Text->SetVisibility(ESlateVisibility::Collapsed);
                    InGameWidget->SetScore(Score);
                }
                if (ASnakeWorld* W = Cast<ASnakeWorld>(
                        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())))
                {
                    InGameWidget->SetLevel(W->LevelIndex);
                }
                else
                {
                    InGameWidget->SetLevel(1);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create InGameWidget from %s"), *GetNameSafe(InGameWidgetClass));
            }
        }
        break;

    case EGameState::Pause:
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        if (PauseMenuWidgetClass)
        {
            auto* UW = CreateWidget<UMyUserWidget>(GetWorld(), PauseMenuWidgetClass);
            PauseWidget = UW;
            if (UW)
            {
                UW->AddToViewport();
                if (ASnakeWorld* W = Cast<ASnakeWorld>(
                        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())))
                {
                    UW->SetLevel(W->LevelIndex);
                }
                if (CurrentGameType == EGameType::PvP || CurrentGameType == EGameType::PvAI)
                {
                    UW->ScoreText  ->SetVisibility(ESlateVisibility::Collapsed);
                    UW->ScoreP1Text->SetVisibility(ESlateVisibility::Visible);
                    UW->ScoreP2Text->SetVisibility(ESlateVisibility::Visible);
                    UW->SetPlayerScores(ApplesEatenP1, ApplesEatenP2);
                }
                else
                {
                    UW->ScoreText  ->SetVisibility(ESlateVisibility::Visible);
                    UW->ScoreP1Text->SetVisibility(ESlateVisibility::Collapsed);
                    UW->ScoreP2Text->SetVisibility(ESlateVisibility::Collapsed);
                    UW->SetScore(Score);
                }
            }
        }
        break;

    case EGameState::Outro:
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        if (GameOverWidgetClass)
        {
            auto* UW = CreateWidget<UMyUserWidget>(GetWorld(), GameOverWidgetClass);
            CurrentWidget = UW;
            if (UW)
            {
                UW->AddToViewport();
                if (ASnakeWorld* W = Cast<ASnakeWorld>(
                        UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())))
                {
                    UW->SetLevel(W->LevelIndex);
                }
                if (CurrentGameType == EGameType::PvP || CurrentGameType == EGameType::PvAI)
                {
                    UW->ScoreText  ->SetVisibility(ESlateVisibility::Collapsed);
                    UW->ScoreP1Text->SetVisibility(ESlateVisibility::Visible);
                    UW->ScoreP2Text->SetVisibility(ESlateVisibility::Visible);
                    UW->SetPlayerScores(ApplesEatenP1, ApplesEatenP2);
                }
                else
                {
                    UW->ScoreText  ->SetVisibility(ESlateVisibility::Visible);
                    UW->ScoreP1Text->SetVisibility(ESlateVisibility::Collapsed);
                    UW->ScoreP2Text->SetVisibility(ESlateVisibility::Collapsed);
                    UW->SetScore(Score);
                }
            }
        }
        break;
    }
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

void ASnakeGameMode::RestartGame()
{
    UWorld* W = GetWorld();
    if (!W) return;

    // Get map name without prefix
    FString MapName = W->GetMapName();
    MapName.RemoveFromStart(W->StreamingLevelsPrefix);

    // Open it again
    UGameplayStatics::OpenLevel(W, FName(*MapName));
}