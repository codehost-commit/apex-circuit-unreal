#pragma once

#include "CoreMinimal.h"
#include "ApexCircuitTypes.generated.h"

UENUM(BlueprintType)
enum class EApexSurface : uint8
{
	Asphalt,
	Kerb,
	Grass,
	Gravel
};

USTRUCT(BlueprintType)
struct FApexDrsZone
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float StartDistanceCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float EndDistanceCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DetectionDistanceCm = 0.0f;
};

USTRUCT(BlueprintType)
struct FApexSurfaceSample
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EApexSurface Surface = EApexSurface::Asphalt;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Grip = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RollingResistance = 0.012f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LateralOffsetCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProgressCm = 0.0f;
};
