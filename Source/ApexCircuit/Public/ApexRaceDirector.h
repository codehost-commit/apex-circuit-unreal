#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApexCircuitTypes.h"
#include "ApexRaceDirector.generated.h"

class AApexFormulaCar;
class AApexTrackActor;
class UApexSessionConfigDataAsset;

struct FApexCarRaceState
{
	float PreviousProgressCm = 0.0f;
	float CurrentLapSeconds = 0.0f;
	float LastLapSeconds = 0.0f;
	float BestLapSeconds = TNumericLimits<float>::Max();
	float PenaltySeconds = 0.0f;
	float OffTrackSeconds = 0.0f;
	float SafeProgressCm = 0.0f;
	int32 Lap = 1;
	int32 Sector = 1;
	int32 NextCheckpoint = 0;
	bool bLapValid = true;
	bool bFinished = false;
	bool bInitialized = false;
};

UCLASS()
class APEXCIRCUIT_API AApexRaceDirector : public AActor
{
	GENERATED_BODY()

public:
	AApexRaceDirector();
	virtual void Tick(float DeltaSeconds) override;
	void Initialize(AApexTrackActor* InTrack, AApexFormulaCar* InPlayerCar);
	void SetField(const TArray<AApexFormulaCar*>& InField);

	UFUNCTION(BlueprintCallable, Category = "Apex|Race") void StartSession(EApexSessionMode InMode);
	UFUNCTION(BlueprintCallable, Category = "Apex|Race") void RestartSession();
	UFUNCTION(BlueprintCallable, Category = "Apex|Race") void TogglePause();
	UFUNCTION(BlueprintCallable, Category = "Apex|Race") void RecoverPlayer();
	UFUNCTION(BlueprintPure, Category = "Apex|Race") int32 GetCurrentLap() const;
	UFUNCTION(BlueprintPure, Category = "Apex|Race") int32 GetCurrentSector() const;
	UFUNCTION(BlueprintPure, Category = "Apex|Race") float GetSessionTimeSeconds() const { return SessionTimeSeconds; }
	UFUNCTION(BlueprintPure, Category = "Apex|Race") float GetCurrentLapTimeSeconds() const;
	UFUNCTION(BlueprintPure, Category = "Apex|Race") float GetLastLapTimeSeconds() const;
	UFUNCTION(BlueprintPure, Category = "Apex|Race") float GetProgressCm() const { return CurrentProgressCm; }
	UFUNCTION(BlueprintPure, Category = "Apex|Race") bool IsDrsEligible() const;
	UFUNCTION(BlueprintPure, Category = "Apex|Race") EApexSessionMode GetSessionMode() const { return SessionMode; }
	UFUNCTION(BlueprintPure, Category = "Apex|Race") EApexSessionState GetSessionState() const { return SessionState; }
	UFUNCTION(BlueprintPure, Category = "Apex|Race") int32 GetStartLights() const { return StartLights; }
	UFUNCTION(BlueprintPure, Category = "Apex|Race") int32 GetSessionLapTarget() const { return SessionLapTarget; }
	UFUNCTION(BlueprintPure, Category = "Apex|Race") bool IsCurrentLapValid() const;
	UFUNCTION(BlueprintPure, Category = "Apex|Race") FString GetRaceMessage() const { return RaceMessage; }
	const TArray<FApexClassificationRow>& GetClassification() const { return Classification; }

private:
	static float ForwardDistance(float FromCm, float ToCm, float TrackLengthCm);
	static bool CrossedForwardDistance(float FromCm, float ToCm, float TargetCm, float TrackLengthCm);
	FApexCarRaceState& StateFor(AApexFormulaCar* Car);
	void ResetFieldForSession();
	void UpdateStartSequence(float DeltaSeconds);
	void UpdateCarProgress(AApexFormulaCar* Car, float DeltaSeconds);
	void UpdateClassification();
	void SetMessage(const FString& InMessage);
	void UpdateDrsForCar(AApexFormulaCar* Car, const FApexCarRaceState& State);

	UPROPERTY(EditAnywhere, Category = "Apex|Race") TObjectPtr<UApexSessionConfigDataAsset> SessionConfig;
	UPROPERTY(Transient) TObjectPtr<AApexTrackActor> Track;
	UPROPERTY(Transient) TObjectPtr<AApexFormulaCar> PlayerCar;
	UPROPERTY(Transient) TArray<TObjectPtr<AApexFormulaCar>> Field;
	TMap<TObjectPtr<AApexFormulaCar>, FApexCarRaceState> CarStates;
	TArray<FApexClassificationRow> Classification;
	EApexSessionMode SessionMode = EApexSessionMode::Menu;
	EApexSessionState SessionState = EApexSessionState::Menu;
	float SessionTimeSeconds = 0.0f;
	float CurrentProgressCm = 0.0f;
	float StartTimerSeconds = 0.0f;
	float NextLightSeconds = 0.0f;
	int32 StartLights = 0;
	int32 SessionLapTarget = 0;
	FString RaceMessage = TEXT("SELECT A SESSION");
	FRandomStream Random{2026};
};
