#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnakeGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	MainMenu    UMETA(DisplayName="Main Menu"),
	Game        UMETA(DisplayName="Game"),
	Pause       UMETA(DisplayName="Pause"),
	Outro       UMETA(DisplayName="Outro")
};

UENUM(BlueprintType)
enum class EGameType : uint8
{
	SinglePlayer    UMETA(DisplayName="Single Player"),
	PvP             UMETA(DisplayName="Player vs Player"),
	Coop            UMETA(DisplayName="Cooperative"),
	PvAI            UMETA(DisplayName="Player vs AI"),
	CoopAI          UMETA(DisplayName="Cooperative + AI")
};

UCLASS()
class SNAKEGAME_API ASnakeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnakeGameMode();

	virtual void BeginPlay() override;

	// --- New property for game type ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game Type")
	EGameType CurrentGameType = EGameType::SinglePlayer;

	// Call this from your MainMenu widget stubs
	UFUNCTION(BlueprintCallable, Category="Game Type")
	void SetGameType(EGameType NewType)
	{
		CurrentGameType = NewType;
		UE_LOG(LogTemp, Log, TEXT("GameType set to %s"), *UEnum::GetValueAsString(NewType));
	}

	// Pause menu widget class
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	// Function to change the game state and update the menu UI accordingly.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGameState(EGameState NewState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
	EGameState CurrentState;

	// Widget classes you’ll assign in the editor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;

	// How many apples must be eaten before advancing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	int32 ApplesToFinish = 5;

	// How many apples have been eaten so far
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Level")
	int32 ApplesEaten = 0;

	// Call this whenever the snake eats an apple
	UFUNCTION()
	void NotifyAppleEaten();

protected:
	// Currently displayed UI widget (if any).
	UPROPERTY()
	UUserWidget* CurrentWidget;

private:
	UUserWidget* PauseWidget = nullptr;
};
