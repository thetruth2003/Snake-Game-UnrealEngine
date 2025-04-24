// SnakeGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "SnakePawn.h"                // for ASnakePawn
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
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // --- Game type ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game Type")
    EGameType CurrentGameType = EGameType::SinglePlayer;

    UFUNCTION(BlueprintCallable, Category="Game Type")
    void SetGameType(EGameType NewType);

    // ** Two separate pawn Blueprints for each player **
    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> Player1PawnBP;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> Player2PawnBP;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> AISnakePawnBP;

    // UI widget classes
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    // Level progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 ApplesToFinish = 5;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Level")
    int32 ApplesEaten = 0;

    UFUNCTION()
    void NotifyAppleEaten();

    // Game state machine
    UFUNCTION(BlueprintCallable, Category="Game State")
    void SetGameState(EGameState NewState);

    UFUNCTION(BlueprintCallable, Category="Game State")
    EGameState GetCurrentState() const { return CurrentState; }

protected:
    UPROPERTY()
    UUserWidget* CurrentWidget;

private:
    // Private game state; use GetCurrentState() to query
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game State", meta=(AllowPrivateAccess="true"))
    EGameState CurrentState = EGameState::MainMenu;

    // track the AI snake pawn so we don’t spawn it twice
    UPROPERTY()
    ASnakePawn* SpawnedAISnake = nullptr;

    UUserWidget* PauseWidget = nullptr;
};
