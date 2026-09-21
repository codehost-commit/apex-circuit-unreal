#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApexRaceDirector.generated.h"

class AApexFormulaCar;
class AApexTrackActor;

UCLASS()
class APEXCIRCUIT_API AApexRaceDirector : public AActor
{
	GENERATED_BODY()

public:
	AApexRaceDirector();

	virtual void Tick(float DeltaSeconds) override;

	void Initialize(AApexTrackActor* InTrack, AApexFormulaCar* InPlayerCar);

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	int32 GetCurrentLap() const { return CurrentLap; }

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	int32 GetCurrentSector() const { return CurrentSector; }

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	float GetSessionTimeSeconds() const { return SessionTimeSeconds; }

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	float GetCurrentLapTimeSeconds() const { return CurrentLapTimeSeconds; }

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	float GetLastLapTimeSeconds() const { return LastLapTimeSeconds; }

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	float GetProgressCm() const { return CurrentProgressCm; }

	UFUNCTION(BlueprintPure, Category = "Apex|Race")
	bool IsDrsEligible() const { return bDrsArmed; }

private:
	static float ForwardDistance(float FromCm, float ToCm, float TrackLengthCm);
	static bool CrossedForwardDistance(float FromCm, float ToCm, float TargetCm, float TrackLengthCm);
	void UpdateRaceState(float DeltaSeconds);

	TWeakObjectPtr<AApexTrackActor> Track;
	TWeakObjectPtr<AApexFormulaCar> PlayerCar;
	bool bHasPreviousProgress = false;
	bool bDrsArmed = false;
	bool bWasInDrsZone = false;
	int32 NextCheckpointIndex = 0;
	int32 CurrentLap = 1;
	int32 CurrentSector = 1;
	float SessionTimeSeconds = 0.0f;
	float CurrentLapTimeSeconds = 0.0f;
	float LastLapTimeSeconds = 0.0f;
	float CurrentProgressCm = 0.0f;
	float PreviousProgressCm = 0.0f;
};
