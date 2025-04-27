
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

class UMyUserWidget;

UCLASS()
class SNAKEGAME_API ASnakeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASnakeGameMode();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UMyUserWidget> InGameWidgetClass;

    UPROPERTY()
    UMyUserWidget* InGameWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> Player1PawnBP;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> Player2PawnBP;

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ASnakePawn> AISnakePawnBP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 ApplesToFinish = 5;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Level")
    int32 ApplesEaten = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game")
    int32 Score = 0;

    UFUNCTION()
    void NotifyAppleEaten(int32 ControllerId);

    UFUNCTION(BlueprintCallable, Category="Game State")
    void SetGameState(EGameState NewState);

    UFUNCTION(BlueprintCallable, Category="Game State")
    EGameState GetCurrentState() const { return CurrentState; }

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

    UPROPERTY()
    ASnakePawn* SpawnedAISnake = nullptr;

    int32 ApplesEatenP1 = 0;
    int32 ApplesEatenP2 = 0;
};