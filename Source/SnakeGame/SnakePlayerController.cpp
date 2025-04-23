// SnakePlayerController.cpp
#include "SnakePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

void ASnakePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		const int32 Id = LP->GetControllerId();
		UInputMappingContext* ToApply = (Id == 0) ? P1Mapping : P2Mapping;
		if (ToApply)
			Subsystem->AddMappingContext(ToApply, 0);
	}
}
