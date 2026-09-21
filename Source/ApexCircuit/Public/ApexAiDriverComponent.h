#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ApexAiDriverComponent.generated.h"

class AApexFormulaCar;
class AApexTrackActor;

/** Fixed-rate, deterministic production driver. It follows the real circuit spline and is intentionally independent of experimental ML code. */
UCLASS(ClassGroup = (Apex), BlueprintType, meta = (BlueprintSpawnableComponent))
class APEXCIRCUIT_API UApexAiDriverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UApexAiDriverComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Initialize(AApexTrackActor* InTrack, float InPaceScale, int32 InSeed);

private:
	float ComputeTargetSpeed(float ProgressCm) const;
	float ControlAccumulator = 0.0f;
	float PaceScale = 1.0f;
	FRandomStream Random;
	TWeakObjectPtr<AApexFormulaCar> Car;
	TWeakObjectPtr<AApexTrackActor> Track;
};
