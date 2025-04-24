// SnakeGameMode.cpp
#include "SnakeGameMode.h"
#include "SnakeWorld.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"

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
    UE_LOG(LogTemp, Log, TEXT("GameType set to %s"), *UEnum::GetValueAsString(NewType));

    // Add second local player when needed
    if ((NewType == EGameType::Coop || NewType == EGameType::PvP)
        && GetGameInstance()->GetNumLocalPlayers() < 2)
    {
        UGameplayStatics::CreatePlayer(GetWorld(), 1, true);
    }

    // Enter gameplay
    SetGameState(EGameState::Game);
}

void ASnakeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 Id = NewPlayer->GetLocalPlayer()->GetControllerId();
    if (Id == 1 && (CurrentGameType == EGameType::Coop || CurrentGameType == EGameType::PvP))
    {
        FVector SpawnLoc(400,1000,0);
        FActorSpawnParameters Params;
        // spawn BP_SnakePawn2 only
        if (Player2PawnBP)
        {
            ASnakePawn* Pawn = GetWorld()->SpawnActor<ASnakePawn>(
                Player2PawnBP, SpawnLoc, FRotator::ZeroRotator, Params);
            if (Pawn) NewPlayer->Possess(Pawn);
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
