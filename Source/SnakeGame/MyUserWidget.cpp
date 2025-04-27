#include "MyUserWidget.h"

void UMyUserWidget::SetScore(int32 InScore)
{
	if (ScoreText)
		ScoreText->SetText(FText::AsNumber(InScore));
}

void UMyUserWidget::SetLevel(int32 InLevel)
{
	if (LevelText)
		LevelText->SetText(FText::AsNumber(InLevel));
}

void UMyUserWidget::SetPlayerScores(int32 InP1Score, int32 InP2Score)
{
	if (ScoreP1Text)
	{
		FString P1Str = FString::Printf(TEXT("P1: %d"), InP1Score);
		ScoreP1Text->SetText(FText::FromString(P1Str));
	}

	if (ScoreP2Text)
	{
		FString P2Str = FString::Printf(TEXT("P2: %d"), InP2Score);
		ScoreP2Text->SetText(FText::FromString(P2Str));
	}
}
