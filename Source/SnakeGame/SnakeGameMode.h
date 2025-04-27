// SnakeGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "SnakePawn.h"
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

// Forward‐declare the UMG widget class so we can use TSubclassOf<UMyUserWidget>
class UMyUserWidget;

UCLASS()
class SNAKEGAME_API ASnakeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASnakeGameMode();

    // In‐Game HUD widget blueprint
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UMyUserWidget> InGameWidgetClass;

    // Live instance of the in‐game HUD
    UPROPERTY()
    UMyUserWidget* InGameWidget = nullptr;

    // Main menu, pause and game‐over widgets
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    // Two‐player and AI spawning
    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> Player1PawnBP;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> Player2PawnBP;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> AISnakePawnBP;

    // Level & progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 ApplesToFinish = 5;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Level")
    int32 ApplesEaten = 0;

    // Cumulative score
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game")
    int32 Score = 0;

    UFUNCTION()
    void NotifyAppleEaten(int32 ControllerId);

    // Game state machine
    UFUNCTION(BlueprintCallable, Category="Game State")
    void SetGameState(EGameState NewState);

    UFUNCTION(BlueprintCallable, Category="Game State")
    EGameState GetCurrentState() const { return CurrentState; }

    // Switch between Single, PvP, Coop, etc.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game Type")
    EGameType CurrentGameType = EGameType::SinglePlayer;

    UFUNCTION(BlueprintCallable, Category="Game Type")
    void SetGameType(EGameType NewType);

    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UFUNCTION(BlueprintCallable, Category="Game")
    void RestartGame();

protected:
    UPROPERTY()
    UUserWidget* CurrentWidget;

    UPROPERTY()
    UUserWidget* PauseWidget;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
    EGameState CurrentState = EGameState::MainMenu;

    // Track AI snake pawn
    UPROPERTY()
    ASnakePawn* SpawnedAISnake = nullptr;

    // Per‐player apples for versus modes
    int32 ApplesEatenP1 = 0;
    int32 ApplesEatenP2 = 0;
};
