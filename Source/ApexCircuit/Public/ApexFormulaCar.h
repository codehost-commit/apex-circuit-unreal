#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ApexCircuitTypes.h"
#include "ApexFormulaCar.generated.h"

class AApexRaceDirector;
class AApexTrackActor;
class UApexCarTuningDataAsset;
class UBoxComponent;
class UAudioComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UStaticMeshComponent;
class UApexTelemetryComponent;
struct FInputActionValue;

struct FApexWheelState
{
	FVector LocalMountCm = FVector::ZeroVector;
	bool bFront = false;
	bool bGrounded = false;
	float CompressionCm = 0.0f;
	float NormalLoadNewton = 0.0f;
	EApexSurface Surface = EApexSurface::Asphalt;
};

UCLASS()
class APEXCIRCUIT_API AApexFormulaCar : public APawn
{
	GENERATED_BODY()

public:
	AApexFormulaCar();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PawnClientRestart() override;

	void SetTrack(AApexTrackActor* InTrack);
	void SetRaceDirector(AApexRaceDirector* InRaceDirector);
	void SetDrsAvailability(bool bInDrsAvailability);
	void SetSafeProgressCm(float InSafeProgressCm);
	void SetRaceEnabled(bool bEnabled);
	void SetAiControl(float InThrottle, float InBrake, float InSteering, bool bInDrs, bool bInErs);
	void SetDriverIdentity(const FString& InName, int32 InNumber);
	void ResetToTransform(const FTransform& Transform);

	UFUNCTION(BlueprintCallable, Category = "Apex|Car")
	void ResetVehicle();

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetSpeedMps() const { return SpeedMps; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetSpeedKph() const { return SpeedMps * 3.6f; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetEngineRpm() const { return EngineRpm; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	int32 GetGear() const { return CurrentGear; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	bool IsDrsOpen() const { return bDrsOpen; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	bool IsDrsAvailable() const { return bDrsAvailable; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetThrottleInput() const { return ThrottleInput; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetBrakeInput() const { return BrakeInput; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetSteeringInput() const { return SteeringInput; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	float GetEnergyMj() const { return EnergyMj; }

	UFUNCTION(BlueprintPure, Category = "Apex|Car")
	EApexSurface GetCurrentSurface() const { return CurrentSurface; }
	bool IsRaceEnabled() const { return bRaceEnabled; }
	const FString& GetDriverName() const { return DriverName; }
	int32 GetCarNumber() const { return CarNumber; }

	const TArray<FApexWheelState>& GetWheelStates() const { return Wheels; }
	UApexTelemetryComponent* GetTelemetryComponent() const { return Telemetry; }

private:
	void AddInputMapping();
	void ConfigureInputActions();
	UApexCarTuningDataAsset* ResolveTuning();
	void ApplyVehicleForces(float DeltaSeconds);
	void UpdatePowertrain(float DeltaSeconds, float ForwardSpeedMps);
	void UpdateTelemetry(float DeltaSeconds);
	void UpdateWheelVisuals();
	void UpdateEngineAudio();
	void SetActiveCamera(int32 CameraIndex);
	void Turn(float Value);
	void LookUp(float Value);
	void SetThrottle(const FInputActionValue& Value);
	void SetBrake(const FInputActionValue& Value);
	void SetSteering(const FInputActionValue& Value);
	void SetDrs(const FInputActionValue& Value);
	void SetErs(const FInputActionValue& Value);
	void ToggleCamera(const FInputActionValue& Value);
	void TriggerReset(const FInputActionValue& Value);
	void TogglePause(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<UBoxComponent> Chassis;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<UStaticMeshComponent> ChassisVisual;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TArray<TObjectPtr<UStaticMeshComponent>> WheelVisuals;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<USpringArmComponent> ChaseBoom;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<UCameraComponent> ChaseCamera;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<USpringArmComponent> TCamBoom;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<UCameraComponent> TCamCamera;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<UCameraComponent> CockpitCamera;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Car")
	TObjectPtr<UApexTelemetryComponent> Telemetry;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Audio")
	TObjectPtr<UAudioComponent> EngineLowAudio;
	UPROPERTY(VisibleAnywhere, Category = "Apex|Audio")
	TObjectPtr<UAudioComponent> EngineMidAudio;
	UPROPERTY(VisibleAnywhere, Category = "Apex|Audio")
	TObjectPtr<UAudioComponent> EngineHighAudio;

	UPROPERTY(EditAnywhere, Category = "Apex|Car")
	TObjectPtr<UApexCarTuningDataAsset> TuningAsset;

	UPROPERTY(Transient)
	TObjectPtr<UApexCarTuningDataAsset> RuntimeFallbackTuning;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> DrivingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ThrottleAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> SteeringAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> DrsAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ErsAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> CameraAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ResetAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PauseAction;

	TWeakObjectPtr<AApexTrackActor> Track;
	TWeakObjectPtr<AApexRaceDirector> RaceDirector;
	TArray<FApexWheelState> Wheels;
	float ThrottleInput = 0.0f;
	float BrakeInput = 0.0f;
	float SteeringInput = 0.0f;
	float SteeringAngleDegrees = 0.0f;
	float SpeedMps = 0.0f;
	float EngineRpm = 5000.0f;
	float EnergyMj = 4.0f;
	float SafeProgressCm = 0.0f;
	float TelemetryAccumulator = 0.0f;
	int32 CurrentGear = 1;
	int32 ActiveCameraIndex = 0;
	bool bDrsInput = false;
	bool bErsInput = false;
	bool bDrsAvailable = false;
	bool bDrsOpen = false;
	bool bRaceEnabled = true;
	bool bAiControlled = false;
	FString DriverName = TEXT("PLAYER");
	int32 CarNumber = 1;
	EApexSurface CurrentSurface = EApexSurface::Asphalt;
};
