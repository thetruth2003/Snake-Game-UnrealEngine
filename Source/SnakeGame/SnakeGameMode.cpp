#include "SnakeGameMode.h"
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
