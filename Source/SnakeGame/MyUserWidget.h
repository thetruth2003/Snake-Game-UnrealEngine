#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MyUserWidget.generated.h"

/**
 * UI widget for displaying Score, Level, and (optionally) Versus scores.
 */
UCLASS()
class SNAKEGAME_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Bind to a TextBlock named 'ScoreText' in the UMG Designer
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreText;

	// Bind to a TextBlock named 'LevelText'
	UPROPERTY(meta=(BindWidget))
	UTextBlock* LevelText;

	// Optional: Bind to 'ScoreP1Text' for player 1 in Versus modes
	UPROPERTY(meta=(BindWidget, OptionalWidget))
	UTextBlock* ScoreP1Text;

	// Optional: Bind to 'ScoreP2Text' for player 2/AI in Versus modes
	UPROPERTY(meta=(BindWidget, OptionalWidget))
	UTextBlock* ScoreP2Text;
	
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetScore(int32 InScore);

	UFUNCTION(BlueprintCallable, Category="UI")
	void SetLevel(int32 InLevel);

	UFUNCTION(BlueprintCallable, Category="UI")
	void SetPlayerScores(int32 InP1Score, int32 InP2Score);
};