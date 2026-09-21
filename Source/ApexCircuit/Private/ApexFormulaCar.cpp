#include "ApexFormulaCar.h"

#include "ApexCarTuningDataAsset.h"
#include "ApexRaceDirector.h"
#include "ApexTelemetryComponent.h"
#include "ApexTrackActor.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float GravityMps2 = 9.81f;
	constexpr float NewtonToUnrealForce = 100.0f;
}

AApexFormulaCar::AApexFormulaCar()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	Chassis = CreateDefaultSubobject<UBoxComponent>(TEXT("Chassis"));
	Chassis->SetBoxExtent(FVector(235.0f, 90.0f, 28.0f));
	Chassis->SetCollisionProfileName(TEXT("PhysicsActor"));
	Chassis->SetSimulatePhysics(true);
	Chassis->SetEnableGravity(true);
	Chassis->SetLinearDamping(0.03f);
	Chassis->SetAngularDamping(0.28f);
	Chassis->BodyInstance.bUseCCD = true;
	SetRootComponent(Chassis);

	ChassisVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisVisual"));
	ChassisVisual->SetupAttachment(Chassis);
	ChassisVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ChassisVisual->SetRelativeScale3D(FVector(4.7f, 1.8f, 0.35f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CubeMesh.Succeeded())
	{
		ChassisVisual->SetStaticMesh(CubeMesh.Object);
	}

	Wheels = {
		{FVector(162.0f, -95.0f, 0.0f), true},
		{FVector(162.0f, 95.0f, 0.0f), true},
		{FVector(-183.0f, -95.0f, 0.0f), false},
		{FVector(-183.0f, 95.0f, 0.0f), false}
	};
	for (int32 Index = 0; Index < Wheels.Num(); ++Index)
	{
		UStaticMeshComponent* WheelVisual = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("WheelVisual%d"), Index));
		WheelVisual->SetupAttachment(Chassis);
		WheelVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WheelVisual->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		WheelVisual->SetRelativeScale3D(FVector(0.68f, 0.68f, 0.25f));
		if (CylinderMesh.Succeeded())
		{
			WheelVisual->SetStaticMesh(CylinderMesh.Object);
		}
		WheelVisuals.Add(WheelVisual);
	}

	ChaseBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseBoom"));
	ChaseBoom->SetupAttachment(Chassis);
	ChaseBoom->TargetArmLength = 760.0f;
	ChaseBoom->SetRelativeLocation(FVector(-40.0f, 0.0f, 125.0f));
	ChaseBoom->SetRelativeRotation(FRotator(-9.0f, 0.0f, 0.0f));
	ChaseBoom->bUsePawnControlRotation = true;
	ChaseBoom->bEnableCameraLag = true;
	ChaseBoom->CameraLagSpeed = 10.0f;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(ChaseBoom, USpringArmComponent::SocketName);
	ChaseCamera->bUsePawnControlRotation = false;

	TCamBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("TCamBoom"));
	TCamBoom->SetupAttachment(Chassis);
	TCamBoom->TargetArmLength = 280.0f;
	TCamBoom->SetRelativeLocation(FVector(-110.0f, 0.0f, 150.0f));
	TCamBoom->bUsePawnControlRotation = true;

	TCamCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TCamCamera"));
	TCamCamera->SetupAttachment(TCamBoom, USpringArmComponent::SocketName);
	TCamCamera->bUsePawnControlRotation = false;

	CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
	CockpitCamera->SetupAttachment(Chassis);
	CockpitCamera->SetRelativeLocation(FVector(105.0f, 0.0f, 65.0f));
	CockpitCamera->bUsePawnControlRotation = true;

	Telemetry = CreateDefaultSubobject<UApexTelemetryComponent>(TEXT("Telemetry"));

	static ConstructorHelpers::FObjectFinder<UApexCarTuningDataAsset> TuningFinder(TEXT("/Game/Data/DA_ApexFormulaCarTuning.DA_ApexFormulaCarTuning"));
	if (TuningFinder.Succeeded())
	{
		TuningAsset = TuningFinder.Object;
	}

	DrivingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_ApexDriveRuntime"));
	ThrottleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ThrottleRuntime"));
	BrakeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_BrakeRuntime"));
	SteeringAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_SteeringRuntime"));
	DrsAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DRSRuntime"));
	ErsAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ERSRuntime"));
	CameraAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_CameraRuntime"));
	ResetAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ResetRuntime"));
	PauseAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_PauseRuntime"));

	ThrottleAction->ValueType = EInputActionValueType::Axis1D;
	BrakeAction->ValueType = EInputActionValueType::Axis1D;
	SteeringAction->ValueType = EInputActionValueType::Axis1D;
	DrsAction->ValueType = EInputActionValueType::Boolean;
	ErsAction->ValueType = EInputActionValueType::Boolean;
	CameraAction->ValueType = EInputActionValueType::Boolean;
	ResetAction->ValueType = EInputActionValueType::Boolean;
	PauseAction->ValueType = EInputActionValueType::Boolean;

	SetActiveCamera(0);
}

void AApexFormulaCar::BeginPlay()
{
	Super::BeginPlay();
	const UApexCarTuningDataAsset* Tuning = ResolveTuning();
	Chassis->SetMassOverrideInKg(NAME_None, Tuning->MassKg, true);
	Chassis->SetCenterOfMass(Tuning->CenterOfMassOffsetCm);
	EngineRpm = Tuning->IdleRpm;
	ConfigureInputActions();
	UpdateWheelVisuals();
	UE_LOG(LogTemp, Display, TEXT("APEX Formula Car initialized: %.0f kg, %.0f hp-equivalent ICE target, %d forward gears."), Tuning->MassKg, Tuning->PeakPowerWatts / 745.7f, Tuning->GearRatios.Num());
}

void AApexFormulaCar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ApplyVehicleForces(DeltaSeconds);
	UpdateWheelVisuals();
	UpdateTelemetry(DeltaSeconds);
}

void AApexFormulaCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AApexFormulaCar::SetThrottle);
		EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AApexFormulaCar::SetThrottle);
		EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AApexFormulaCar::SetBrake);
		EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AApexFormulaCar::SetBrake);
		EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AApexFormulaCar::SetSteering);
		EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AApexFormulaCar::SetSteering);
		EnhancedInput->BindAction(DrsAction, ETriggerEvent::Triggered, this, &AApexFormulaCar::SetDrs);
		EnhancedInput->BindAction(DrsAction, ETriggerEvent::Completed, this, &AApexFormulaCar::SetDrs);
		EnhancedInput->BindAction(ErsAction, ETriggerEvent::Triggered, this, &AApexFormulaCar::SetErs);
		EnhancedInput->BindAction(ErsAction, ETriggerEvent::Completed, this, &AApexFormulaCar::SetErs);
		EnhancedInput->BindAction(CameraAction, ETriggerEvent::Started, this, &AApexFormulaCar::ToggleCamera);
		EnhancedInput->BindAction(ResetAction, ETriggerEvent::Started, this, &AApexFormulaCar::TriggerReset);
		EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AApexFormulaCar::TogglePause);
	}

	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AApexFormulaCar::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AApexFormulaCar::LookUp);
}

void AApexFormulaCar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AddInputMapping();
}

void AApexFormulaCar::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddInputMapping();
}

void AApexFormulaCar::SetTrack(AApexTrackActor* InTrack)
{
	Track = InTrack;
}

void AApexFormulaCar::SetRaceDirector(AApexRaceDirector* InRaceDirector)
{
	RaceDirector = InRaceDirector;
}

void AApexFormulaCar::SetDrsAvailability(bool bInDrsAvailability)
{
	bDrsAvailable = bInDrsAvailability;
}

void AApexFormulaCar::SetSafeProgressCm(float InSafeProgressCm)
{
	SafeProgressCm = InSafeProgressCm;
}

void AApexFormulaCar::ResetVehicle()
{
	if (!Track.IsValid())
	{
		return;
	}

	const FTransform ResetTransform = Track->GetSpawnTransformAtDistance(SafeProgressCm);
	Chassis->SetSimulatePhysics(false);
	SetActorTransform(ResetTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Chassis->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Chassis->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	Chassis->SetSimulatePhysics(true);
	CurrentGear = 1;
	EngineRpm = ResolveTuning()->IdleRpm;
	bDrsOpen = false;
}

void AApexFormulaCar::AddInputMapping()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}
	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->RemoveMappingContext(DrivingContext);
			InputSubsystem->AddMappingContext(DrivingContext, 0);
		}
	}
}

void AApexFormulaCar::ConfigureInputActions()
{
	auto LoadAction = [](const TCHAR* Path, TObjectPtr<UInputAction>& Target)
	{
		if (UInputAction* LoadedAction = LoadObject<UInputAction>(nullptr, Path))
		{
			Target = LoadedAction;
		}
	};

	LoadAction(TEXT("/Game/Input/IA_Throttle.IA_Throttle"), ThrottleAction);
	LoadAction(TEXT("/Game/Input/IA_Brake.IA_Brake"), BrakeAction);
	LoadAction(TEXT("/Game/Input/IA_Steering.IA_Steering"), SteeringAction);
	LoadAction(TEXT("/Game/Input/IA_DRS.IA_DRS"), DrsAction);
	LoadAction(TEXT("/Game/Input/IA_ERS.IA_ERS"), ErsAction);
	LoadAction(TEXT("/Game/Input/IA_Camera.IA_Camera"), CameraAction);
	LoadAction(TEXT("/Game/Input/IA_Reset.IA_Reset"), ResetAction);
	LoadAction(TEXT("/Game/Input/IA_Pause.IA_Pause"), PauseAction);

	DrivingContext->UnmapAll();
	DrivingContext->MapKey(ThrottleAction, EKeys::W);
	DrivingContext->MapKey(BrakeAction, EKeys::S);
	DrivingContext->MapKey(BrakeAction, EKeys::SpaceBar);
	DrivingContext->MapKey(SteeringAction, EKeys::D);
	FEnhancedActionKeyMapping& LeftSteeringMapping = DrivingContext->MapKey(SteeringAction, EKeys::A);
	LeftSteeringMapping.Modifiers.Add(NewObject<UInputModifierNegate>(DrivingContext, TEXT("NegateSteering")));
	DrivingContext->MapKey(DrsAction, EKeys::LeftShift);
	DrivingContext->MapKey(ErsAction, EKeys::E);
	DrivingContext->MapKey(CameraAction, EKeys::C);
	DrivingContext->MapKey(ResetAction, EKeys::R);
	DrivingContext->MapKey(PauseAction, EKeys::Escape);
}

UApexCarTuningDataAsset* AApexFormulaCar::ResolveTuning()
{
	if (TuningAsset == nullptr)
	{
		TuningAsset = LoadObject<UApexCarTuningDataAsset>(nullptr, TEXT("/Game/Data/DA_ApexFormulaCarTuning.DA_ApexFormulaCarTuning"));
	}
	if (TuningAsset == nullptr)
	{
		if (RuntimeFallbackTuning == nullptr)
		{
			RuntimeFallbackTuning = NewObject<UApexCarTuningDataAsset>(this, TEXT("FallbackApexTuning"));
		}
		return RuntimeFallbackTuning;
	}
	return TuningAsset;
}

void AApexFormulaCar::ApplyVehicleForces(float DeltaSeconds)
{
	if (!Chassis->IsSimulatingPhysics() || DeltaSeconds <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const UApexCarTuningDataAsset* Tuning = ResolveTuning();
	const FVector ChassisForward = Chassis->GetForwardVector().GetSafeNormal();
	const FVector ChassisUp = Chassis->GetUpVector().GetSafeNormal();
	const FVector VelocityCm = Chassis->GetPhysicsLinearVelocity();
	const float ForwardSpeedMps = FVector::DotProduct(VelocityCm, ChassisForward) / 100.0f;
	SpeedMps = VelocityCm.Length() / 100.0f;

	const float MaxSteerDegrees = FMath::Lerp(Tuning->MaxSteerLowSpeedDegrees, Tuning->MaxSteerHighSpeedDegrees, FMath::Clamp(SpeedMps / 80.0f, 0.0f, 1.0f));
	SteeringAngleDegrees = FMath::FInterpTo(SteeringAngleDegrees, SteeringInput * MaxSteerDegrees, DeltaSeconds, Tuning->SteeringResponse * 5.0f);
	UpdatePowertrain(DeltaSeconds, ForwardSpeedMps);

	float AvailableDriveForceN = 0.0f;
	if (CurrentGear > 0 && Tuning->GearRatios.IsValidIndex(CurrentGear - 1) && ForwardSpeedMps < Tuning->MaxSpeedMps)
	{
		const float GearRatio = Tuning->GearRatios[CurrentGear - 1] * Tuning->FinalDrive;
		const float AngularVelocity = FMath::Max(EngineRpm * 2.0f * PI / 60.0f, 1.0f);
		const float TorqueNm = FMath::Min(Tuning->PeakTorqueNm, Tuning->PeakPowerWatts / AngularVelocity);
		AvailableDriveForceN = ThrottleInput * FMath::Min3(Tuning->MaxDriveForceN, TorqueNm * GearRatio * 0.97f / (Tuning->WheelRadiusCm / 100.0f), Tuning->PeakPowerWatts / FMath::Max(7.0f, FMath::Abs(ForwardSpeedMps)));
		if (bErsInput && EnergyMj > 0.0f)
		{
			const float DeployWatts = FMath::Min(120000.0f, EnergyMj * 1000000.0f / DeltaSeconds);
			AvailableDriveForceN += FMath::Min(9500.0f, DeployWatts / FMath::Max(18.0f, FMath::Abs(ForwardSpeedMps)));
			EnergyMj = FMath::Max(0.0f, EnergyMj - DeployWatts * DeltaSeconds / 1000000.0f);
		}
	}

	const float TotalBrakeForceN = BrakeInput * Tuning->MaxBrakeForceN;
	float TotalRollingResistanceN = 0.0f;
	int32 RearContactCount = 0;
	for (const FApexWheelState& Wheel : Wheels)
	{
		if (!Wheel.bFront && Wheel.bGrounded)
		{
			++RearContactCount;
		}
	}

	float SurfaceGripSum = 0.0f;
	int32 SurfaceContactCount = 0;
	for (int32 Index = 0; Index < Wheels.Num(); ++Index)
	{
		FApexWheelState& Wheel = Wheels[Index];
		const FVector Mount = Chassis->GetComponentTransform().TransformPosition(Wheel.LocalMountCm);
		const FVector TraceStart = Mount + ChassisUp * 15.0f;
		const FVector TraceEnd = Mount - ChassisUp * (Tuning->SuspensionRestLengthCm + Tuning->WheelRadiusCm + 60.0f);
		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ApexWheelTrace), false, this);
		QueryParams.AddIgnoredActor(this);

		Wheel.bGrounded = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		Wheel.NormalLoadNewton = 0.0f;
		Wheel.CompressionCm = 0.0f;
		if (!Wheel.bGrounded)
		{
			continue;
		}

		const float MountToSurfaceCm = FMath::Max(0.0f, Hit.Distance - 15.0f);
		const float SpringLengthCm = MountToSurfaceCm - Tuning->WheelRadiusCm;
		Wheel.CompressionCm = FMath::Clamp(Tuning->SuspensionRestLengthCm - SpringLengthCm, 0.0f, Tuning->SuspensionRestLengthCm);
		const FVector ContactVelocityMps = Chassis->GetPhysicsLinearVelocityAtPoint(Hit.ImpactPoint) / 100.0f;
		const float VerticalVelocityMps = FVector::DotProduct(ContactVelocityMps, ChassisUp);
		const float Damping = VerticalVelocityMps < 0.0f ? Tuning->DamperBumpNsPerM : Tuning->DamperReboundNsPerM;
		Wheel.NormalLoadNewton = FMath::Clamp(Tuning->SpringRateNPerM * (Wheel.CompressionCm / 100.0f) - Damping * VerticalVelocityMps, 0.0f, 38000.0f);
		Chassis->AddForceAtLocation(ChassisUp * Wheel.NormalLoadNewton * NewtonToUnrealForce, Hit.ImpactPoint);

		FApexSurfaceSample Surface;
		if (Track.IsValid())
		{
			Surface = Track->SampleSurfaceAtWorldLocation(Hit.ImpactPoint);
		}
		Wheel.Surface = Surface.Surface;
		SurfaceGripSum += Surface.Grip;
		++SurfaceContactCount;
		TotalRollingResistanceN += Surface.RollingResistance * Tuning->MassKg * GravityMps2 * 0.25f;

		const float WheelSteerDegrees = Wheel.bFront ? SteeringAngleDegrees : 0.0f;
		const FVector WheelForward = FQuat(ChassisUp, FMath::DegreesToRadians(WheelSteerDegrees)).RotateVector(ChassisForward).GetSafeNormal();
		const FVector WheelRight = FVector::CrossProduct(ChassisUp, WheelForward).GetSafeNormal();
		const float LongitudinalSpeedMps = FVector::DotProduct(ContactVelocityMps, WheelForward);
		const float LateralSpeedMps = FVector::DotProduct(ContactVelocityMps, WheelRight);
		const float SlipAngle = FMath::Atan2(LateralSpeedMps, FMath::Max(1.0f, FMath::Abs(LongitudinalSpeedMps)));
		const float CorneringStiffness = Wheel.bFront ? Tuning->FrontCorneringStiffness : Tuning->RearCorneringStiffness;
		float LateralForceN = -CorneringStiffness * SlipAngle;
		float LongitudinalForceN = Wheel.bFront ? -TotalBrakeForceN * Tuning->BrakeFrontBias * 0.5f : -TotalBrakeForceN * (1.0f - Tuning->BrakeFrontBias) * 0.5f;
		if (!Wheel.bFront && RearContactCount > 0)
		{
			LongitudinalForceN += AvailableDriveForceN / static_cast<float>(RearContactCount);
		}

		const float FrictionLimitN = Tuning->TyreMu * Surface.Grip * Wheel.NormalLoadNewton;
		const float CombinedForceN = FMath::Sqrt(FMath::Square(LongitudinalForceN) + FMath::Square(LateralForceN));
		if (CombinedForceN > FrictionLimitN && CombinedForceN > KINDA_SMALL_NUMBER)
		{
			const float Scale = FrictionLimitN / CombinedForceN;
			LongitudinalForceN *= Scale;
			LateralForceN *= Scale;
		}
		Chassis->AddForceAtLocation((WheelForward * LongitudinalForceN + WheelRight * LateralForceN) * NewtonToUnrealForce, Hit.ImpactPoint);
	}

	CurrentSurface = SurfaceContactCount > 0 && SurfaceGripSum / static_cast<float>(SurfaceContactCount) < 0.5f ? EApexSurface::Grass : EApexSurface::Asphalt;
	if (SurfaceContactCount > 0)
	{
		for (const FApexWheelState& Wheel : Wheels)
		{
			if (Wheel.bGrounded && Wheel.Surface == EApexSurface::Gravel)
			{
				CurrentSurface = EApexSurface::Gravel;
				break;
			}
			if (Wheel.bGrounded && Wheel.Surface == EApexSurface::Kerb)
			{
				CurrentSurface = EApexSurface::Kerb;
			}
		}
	}

	bDrsOpen = bDrsAvailable && bDrsInput && SpeedMps > 30.0f;
	const float DownforceN = 0.5f * Tuning->AirDensity * Tuning->LiftClArea * SpeedMps * SpeedMps * (bDrsOpen ? Tuning->DrsRearDownforceMultiplier : 1.0f);
	const float DragN = 0.5f * Tuning->AirDensity * Tuning->DragCdArea * SpeedMps * SpeedMps * (bDrsOpen ? Tuning->DrsDragMultiplier : 1.0f);
	Chassis->AddForce(-ChassisUp * DownforceN * NewtonToUnrealForce);
	if (FMath::Abs(ForwardSpeedMps) > 0.1f)
	{
		Chassis->AddForce(-ChassisForward * FMath::Sign(ForwardSpeedMps) * (DragN + TotalRollingResistanceN) * NewtonToUnrealForce);
	}

	if (BrakeInput > 0.01f && SpeedMps > 10.0f)
	{
		EnergyMj = FMath::Min(4.0f, EnergyMj + 70000.0f * BrakeInput * DeltaSeconds / 1000000.0f);
	}
}

void AApexFormulaCar::UpdatePowertrain(float DeltaSeconds, float ForwardSpeedMps)
{
	const UApexCarTuningDataAsset* Tuning = ResolveTuning();
	if (!Tuning->GearRatios.IsValidIndex(CurrentGear - 1))
	{
		CurrentGear = 1;
	}
	const float RadiusM = Tuning->WheelRadiusCm / 100.0f;
	const float Ratio = Tuning->GearRatios[CurrentGear - 1] * Tuning->FinalDrive;
	const float WheelRpm = FMath::Abs(ForwardSpeedMps) / FMath::Max(RadiusM, 0.01f) * Ratio * 60.0f / (2.0f * PI);
	const float LaunchRpm = FMath::Lerp(Tuning->IdleRpm, 9700.0f, ThrottleInput);
	const float TargetRpm = FMath::Clamp(FMath::Max(LaunchRpm, WheelRpm), Tuning->IdleRpm, Tuning->RevLimitRpm);
	EngineRpm = FMath::FInterpTo(EngineRpm, TargetRpm, DeltaSeconds, 30.0f);

	if (EngineRpm >= Tuning->UpshiftRpm && CurrentGear < Tuning->GearRatios.Num())
	{
		++CurrentGear;
	}
	else if (CurrentGear > 1)
	{
		const float LowerRpm = FMath::Abs(ForwardSpeedMps) / FMath::Max(RadiusM, 0.01f) * Tuning->GearRatios[CurrentGear - 2] * Tuning->FinalDrive * 60.0f / (2.0f * PI);
		if (LowerRpm < Tuning->UpshiftRpm * 0.77f)
		{
			--CurrentGear;
		}
	}
}

void AApexFormulaCar::UpdateTelemetry(float DeltaSeconds)
{
	TelemetryAccumulator += DeltaSeconds;
	if (TelemetryAccumulator < 0.1f)
	{
		return;
	}
	TelemetryAccumulator = 0.0f;

	FApexTelemetrySample Sample;
	Sample.SessionTimeSeconds = GetWorld()->GetTimeSeconds();
	Sample.SpeedKph = GetSpeedKph();
	Sample.EngineRpm = EngineRpm;
	Sample.Gear = CurrentGear;
	Sample.Throttle = ThrottleInput;
	Sample.Brake = BrakeInput;
	Sample.Steering = SteeringInput;
	Sample.bDrsOpen = bDrsOpen;
	Sample.Surface = CurrentSurface;
	for (const FApexWheelState& Wheel : Wheels)
	{
		Sample.TyreLoadsNewton.Add(Wheel.NormalLoadNewton);
	}
	if (RaceDirector.IsValid())
	{
		Sample.Lap = RaceDirector->GetCurrentLap();
		Sample.Sector = RaceDirector->GetCurrentSector();
	}
	Telemetry->Record(Sample);
}

void AApexFormulaCar::UpdateWheelVisuals()
{
	const UApexCarTuningDataAsset* Tuning = ResolveTuning();
	for (int32 Index = 0; Index < Wheels.Num() && WheelVisuals.IsValidIndex(Index); ++Index)
	{
		const FApexWheelState& Wheel = Wheels[Index];
		WheelVisuals[Index]->SetRelativeLocation(Wheel.LocalMountCm - FVector(0.0f, 0.0f, Tuning->SuspensionRestLengthCm - Wheel.CompressionCm));
		if (Wheel.bFront)
		{
			WheelVisuals[Index]->SetRelativeRotation(FRotator(90.0f, SteeringAngleDegrees, 0.0f));
		}
	}
}

void AApexFormulaCar::SetActiveCamera(int32 CameraIndex)
{
	ActiveCameraIndex = CameraIndex % 3;
	ChaseCamera->SetActive(ActiveCameraIndex == 0);
	TCamCamera->SetActive(ActiveCameraIndex == 1);
	CockpitCamera->SetActive(ActiveCameraIndex == 2);
}

void AApexFormulaCar::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AApexFormulaCar::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AApexFormulaCar::SetThrottle(const FInputActionValue& Value)
{
	ThrottleInput = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f);
}

void AApexFormulaCar::SetBrake(const FInputActionValue& Value)
{
	BrakeInput = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f);
}

void AApexFormulaCar::SetSteering(const FInputActionValue& Value)
{
	SteeringInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
}

void AApexFormulaCar::SetDrs(const FInputActionValue& Value)
{
	bDrsInput = Value.Get<bool>();
}

void AApexFormulaCar::SetErs(const FInputActionValue& Value)
{
	bErsInput = Value.Get<bool>();
}

void AApexFormulaCar::ToggleCamera(const FInputActionValue& Value)
{
	SetActiveCamera(ActiveCameraIndex + 1);
}

void AApexFormulaCar::TriggerReset(const FInputActionValue& Value)
{
	ResetVehicle();
}

void AApexFormulaCar::TogglePause(const FInputActionValue& Value)
{
	UGameplayStatics::SetGamePaused(this, !UGameplayStatics::IsGamePaused(this));
}
