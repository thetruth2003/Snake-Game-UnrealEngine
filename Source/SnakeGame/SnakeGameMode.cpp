#include "SnakeGameMode.h"

#include "SnakeWorld.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ASnakeGameMode::ASnakeGameMode(): CurrentWidget(nullptr)
{
	// Start in the main menu.
	CurrentState = EGameState::MainMenu;
}

void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	SetGameState(CurrentState);
	if (MainMenuWidgetClass)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
		}
	}

}

void ASnakeGameMode::NotifyAppleEaten()
{
	ApplesEaten++;

	// Find the world actor
	ASnakeWorld* World = Cast<ASnakeWorld>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeWorld::StaticClass())
	);
	if (!World) return;

	if (ApplesEaten < ApplesToFinish)
	{
		// Still in the same level: just spawn another apple
		World->SpawnFood();
	}
	else
	{
		// 1) Pause the game
		SetGameState(EGameState::Pause);

		// 2) Compute what the next level index would be
		int32 NextLevel = World->LevelIndex + 1;

		// 3) If that level file doesn’t exist, go to Game Over:
		if (!World->DoesLevelExist(NextLevel))
		{
			SetGameState(EGameState::Outro);
			return;
		}

		// 4) Otherwise load and resume as before:
		World->LevelIndex = NextLevel;
		World->LoadLevelFromText();
		World->SpawnFood();
		ApplesEaten = 0;
		SetGameState(EGameState::Game);
	}
}

void ASnakeGameMode::SetGameState(EGameState NewState)
{
	// remove current widget if any
	if (CurrentWidget)
	{
		//CurrentWidget->RemoveFromViewport();
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
	}
	
	// remove pause widget if leaving Pause
	if (PauseWidget && NewState != EGameState::Pause)
	{
		//PauseWidget->RemoveFromViewport();
		PauseWidget->RemoveFromParent();
		PauseWidget = nullptr;
	}
	
	CurrentState = NewState;
	
	

	switch (CurrentState)
	{
	case EGameState::MainMenu:
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		if (MainMenuWidgetClass)
		{
			CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
			CurrentWidget->AddToViewport();
		}
		break;

	case EGameState::Game:
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		// (re)load level logic here...
		break;

	case EGameState::Pause:
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		if (PauseMenuWidgetClass)
		{
			PauseWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuWidgetClass);
			PauseWidget->AddToViewport();
		}
		break;

	case EGameState::Outro:
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		if (GameOverWidgetClass)
		{
			CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
			CurrentWidget->AddToViewport();
		}
		break;
	}
}
