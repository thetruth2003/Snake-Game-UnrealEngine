#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MyUserWidget.generated.h"

UCLASS()
class SNAKEGAME_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Bind these to TextBlock widgets in your UMG designer
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;

	// Optional: only present in your VsP widget layouts
	UPROPERTY(meta = (BindWidget, OptionalWidget))
	UTextBlock* ScoreP1Text;

	UPROPERTY(meta = (BindWidget, OptionalWidget))
	UTextBlock* ScoreP2Text;

	// Setter functions to call from C++
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetScore(int32 InScore);

	UFUNCTION(BlueprintCallable, Category="UI")
	void SetLevel(int32 InLevel);

	UFUNCTION(BlueprintCallable, Category="UI")
	void SetPlayerScores(int32 InP1Score, int32 InP2Score);
};