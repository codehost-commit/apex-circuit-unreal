#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ApexCircuitTypes.h"
#include "ApexTelemetryComponent.generated.h"

USTRUCT(BlueprintType)
struct FApexTelemetrySample
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SessionTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SpeedKph = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float EngineRpm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Gear = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Lap = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Sector = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Throttle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Brake = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Steering = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bDrsOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EApexSurface Surface = EApexSurface::Asphalt;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<float> TyreLoadsNewton;
};

UCLASS(ClassGroup = (Apex), BlueprintType, meta = (BlueprintSpawnableComponent))
class APEXCIRCUIT_API UApexTelemetryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UApexTelemetryComponent();

	void Record(const FApexTelemetrySample& Sample);

	UFUNCTION(BlueprintPure, Category = "Apex|Telemetry")
	const FApexTelemetrySample& GetLatestSample() const { return LatestSample; }

	UFUNCTION(BlueprintPure, Category = "Apex|Telemetry")
	const TArray<FApexTelemetrySample>& GetHistory() const { return History; }

private:
	UPROPERTY(EditAnywhere, Category = "Apex|Telemetry")
	int32 MaxHistorySamples = 600;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Telemetry")
	FApexTelemetrySample LatestSample;

	UPROPERTY()
	TArray<FApexTelemetrySample> History;
};
