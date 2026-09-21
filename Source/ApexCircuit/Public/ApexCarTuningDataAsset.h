#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ApexCarTuningDataAsset.generated.h"

UCLASS(BlueprintType)
class APEXCIRCUIT_API UApexCarTuningDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UApexCarTuningDataAsset();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float MassKg = 798.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	FVector CenterOfMassOffsetCm = FVector(0.0f, 0.0f, -20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float YawInertiaKgM2 = 1160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float WheelbaseCm = 345.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float CgToFrontCm = 162.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float FrontTrackCm = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float RearTrackCm = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass and Geometry")
	float WheelRadiusCm = 34.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
	float SuspensionRestLengthCm = 27.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
	float SpringRateNPerM = 142000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
	float DamperBumpNsPerM = 5800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
	float DamperReboundNsPerM = 6900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
	float AntiRollStiffness = 13000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float TyreMu = 2.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float LoadSensitivity = 0.86f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float LongitudinalPeakSlip = 0.115f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float LateralPeakAngleRadians = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float PostPeakFalloff = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float WheelInertiaKgM2 = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float RollingResistance = 0.014f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float OffroadDragForceN = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float FrontCorneringStiffness = 128000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tyres")
	float RearCorneringStiffness = 142000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float IdleRpm = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float RevLimitRpm = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float UpshiftRpm = 18000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float ShiftDurationSeconds = 0.055f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float PeakPowerWatts = 735000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float PeakTorqueNm = 560.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float MaxDriveForceN = 14500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float MaxSpeedMps = 105.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float ReverseSpeedMps = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	float FinalDrive = 3.70f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	TArray<float> GearRatios;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	bool bAutoShift = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float MaxBrakeForceN = 22000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float MaxBrakeTorqueNm = 7480.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float HandbrakeForceN = 9500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float BrakeFrontBias = 0.59f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float MaxSteerLowSpeedDegrees = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float MaxSteerHighSpeedDegrees = 1.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float SteeringTargetAccelerationMps2 = 29.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float SteeringResponse = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float SteeringInputRise = 4.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brakes and Steering")
	float SteeringInputFall = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aerodynamics")
	float DragCdArea = 0.914286f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aerodynamics")
	float LiftClArea = 3.591837f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aerodynamics")
	float FrontAeroBalance = 0.44f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aerodynamics")
	float AirDensity = 1.225f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aerodynamics")
	float DrsDragMultiplier = 0.76f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aerodynamics")
	float DrsRearDownforceMultiplier = 0.68f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bTractionControl = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	bool bAbsEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assists")
	float StabilityAssist = 0.0f;
};
