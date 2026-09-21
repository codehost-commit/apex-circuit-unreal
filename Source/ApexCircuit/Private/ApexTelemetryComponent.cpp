#include "ApexTelemetryComponent.h"

UApexTelemetryComponent::UApexTelemetryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UApexTelemetryComponent::Record(const FApexTelemetrySample& Sample)
{
	LatestSample = Sample;
	History.Add(Sample);
	if (History.Num() > MaxHistorySamples)
	{
		History.RemoveAt(0, History.Num() - MaxHistorySamples, EAllowShrinking::No);
	}
}
