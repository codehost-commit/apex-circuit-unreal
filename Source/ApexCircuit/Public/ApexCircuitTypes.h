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

UENUM(BlueprintType)
enum class EApexSessionMode : uint8
{
	Menu,
	Practice,
	Qualifying,
	TimeTrial,
	Race,
	GPWS,
	Results
};

UENUM(BlueprintType)
enum class EApexSessionState : uint8
{
	Menu,
	Grid,
	Lights,
	Green,
	Paused,
	Finished
};

USTRUCT(BlueprintType)
struct FApexClassificationRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Position = 1;

	UPROPERTY(BlueprintReadOnly)
	FString DriverName;

	UPROPERTY(BlueprintReadOnly)
	int32 CarNumber = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CompletedLaps = 0;

	UPROPERTY(BlueprintReadOnly)
	float BestLapSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float GapSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PenaltySeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bFinished = false;
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
