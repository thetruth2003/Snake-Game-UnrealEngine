#pragma once

#include "CoreMinimal.h"
#include "Components/PointLightComponent.h" 
#include "GameFramework/Actor.h"
#include "SnakeFood.generated.h"

UCLASS()
class SNAKEGAME_API ASnakeFood : public AActor
{
	GENERATED_BODY()

public:    
	ASnakeFood();

	/** The mesh for the apple */  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Glow light attached to the apple */  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light")
	UPointLightComponent* GlowLight;   // ← new
};
