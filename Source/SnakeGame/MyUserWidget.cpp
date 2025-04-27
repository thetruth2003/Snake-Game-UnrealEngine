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
		ScoreP1Text->SetText(FText::AsNumber(InP1Score));
	if (ScoreP2Text)
		ScoreP2Text->SetText(FText::AsNumber(InP2Score));
}
