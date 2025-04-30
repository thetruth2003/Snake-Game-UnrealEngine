#include "SnakeFood.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"  // ← add this

ASnakeFood::ASnakeFood()
{
	PrimaryActorTick.bCanEverTick = false;

	// Apple mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Glow light
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(MeshComponent);
	GlowLight->SetIntensity(3000.0f);                   // brightness—tweak as desired
	GlowLight->SetAttenuationRadius(200.0f);            // how far the glow reaches
	GlowLight->SetLightColor(FLinearColor::Green);      // or whatever color you like
	GlowLight->bUseInverseSquaredFalloff = false;       // makes falloff linear instead of inverse-square
	GlowLight->SetCastShadows(false);                   // disable shadows for performance
}
