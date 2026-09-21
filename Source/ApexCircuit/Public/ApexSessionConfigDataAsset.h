#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ApexSessionConfigDataAsset.generated.h"

/** Authoritative session knobs mirroring the Godot RaceConfig. */
UCLASS(BlueprintType)
class APEXCIRCUIT_API UApexSessionConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
	int32 RaceLaps = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
	int32 QualifyingLaps = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
	float QualifyingDurationSeconds = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
	int32 NormalFieldAiCount = 7;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
	int32 GpwsFieldAiCount = 24;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
	int32 GpwsRaceLaps = 8;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	float DrsGapSeconds = 1.75f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	int32 TrackLimitWarningsBeforePenalty = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	float TrackLimitPenaltySeconds = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	float RecoveryPenaltySeconds = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start")
	float LightStepSeconds = 0.85f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start")
	float MinimumLightsOutSeconds = 2.4f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start")
	float MaximumLightsOutSeconds = 4.8f;
};
