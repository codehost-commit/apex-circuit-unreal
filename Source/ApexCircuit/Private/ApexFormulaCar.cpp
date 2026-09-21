#include "ApexFormulaCar.h"

#include "ApexCarTuningDataAsset.h"
#include "ApexRaceDirector.h"
#include "ApexTelemetryComponent.h"
#include "ApexTrackActor.h"
#include "Components/AudioComponent.h"
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
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
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
	Chassis->SetNotifyRigidBodyCollision(true);
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Rb14Mesh(TEXT("/Game/Vehicles/RB14/rb14/StaticMeshes/SM_RB14.SM_RB14"));
	const bool bUseProxyWheels = !Rb14Mesh.Succeeded();
	if (Rb14Mesh.Succeeded())
	{
		ChassisVisual->SetStaticMesh(Rb14Mesh.Object);
		const FBox Bounds = Rb14Mesh.Object->GetBoundingBox();
		const FVector Size = Bounds.GetSize();
		const bool bLengthAlongY = Size.Y > Size.X;
		const float SourceLength = FMath::Max(bLengthAlongY ? Size.Y : Size.X, 1.0f);
		const float UniformScale = 545.0f / SourceLength;
		ChassisVisual->SetRelativeScale3D(FVector(UniformScale));
		ChassisVisual->SetRelativeRotation(bLengthAlongY ? FRotator(0.0f, -90.0f, 0.0f) : FRotator::ZeroRotator);
		ChassisVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 48.0f - Bounds.GetCenter().Z * UniformScale));
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
		WheelVisual->SetVisibility(bUseProxyWheels, true);
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
	CockpitCamera->SetFieldOfView(82.0f);
	ChaseCamera->SetFieldOfView(76.0f);
	TCamCamera->SetFieldOfView(88.0f);

	Telemetry = CreateDefaultSubobject<UApexTelemetryComponent>(TEXT("Telemetry"));
	EngineLowAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineLowAudio"));
	EngineLowAudio->SetupAttachment(Chassis);
	EngineLowAudio->bAutoActivate = false;
	EngineMidAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineMidAudio"));
	EngineMidAudio->SetupAttachment(Chassis);
	EngineMidAudio->bAutoActivate = false;
	EngineHighAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineHighAudio"));
	EngineHighAudio->SetupAttachment(Chassis);
	EngineHighAudio->bAutoActivate = false;

	WheelSprayFx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WheelSprayFx"));
	WheelSprayFx->SetupAttachment(Chassis);
	WheelSprayFx->SetRelativeLocation(FVector(-190.0f, 0.0f, -24.0f));
	WheelSprayFx->SetRelativeScale3D(FVector(0.75f));
	WheelSprayFx->SetAutoActivate(false);
	TyreBurstFx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TyreBurstFx"));
	TyreBurstFx->SetupAttachment(Chassis);
	TyreBurstFx->SetRelativeLocation(FVector(130.0f, 0.0f, -28.0f));
	TyreBurstFx->SetAutoActivate(false);
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> SpraySystem(TEXT("/Game/FX/NS_ApexWheelSpray.NS_ApexWheelSpray"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BurstSystem(TEXT("/Game/FX/NS_ApexTyreBurst.NS_ApexTyreBurst"));
	if (SpraySystem.Succeeded()) WheelSprayFx->SetAsset(SpraySystem.Object);
	if (BurstSystem.Succeeded()) TyreBurstFx->SetAsset(BurstSystem.Object);

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
	PhotoModeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_PhotoModeRuntime"));
	ReplayAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ReplayRuntime"));
	ToggleAbsAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ToggleAbsRuntime"));
	ToggleTractionAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ToggleTractionRuntime"));
	ErsStrategyAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ErsStrategyRuntime"));

	ThrottleAction->ValueType = EInputActionValueType::Axis1D;
	BrakeAction->ValueType = EInputActionValueType::Axis1D;
	SteeringAction->ValueType = EInputActionValueType::Axis1D;
	DrsAction->ValueType = EInputActionValueType::Boolean;
	ErsAction->ValueType = EInputActionValueType::Boolean;
	CameraAction->ValueType = EInputActionValueType::Boolean;
	ResetAction->ValueType = EInputActionValueType::Boolean;
	PauseAction->ValueType = EInputActionValueType::Boolean;
	PhotoModeAction->ValueType = EInputActionValueType::Boolean;
	PhotoModeAction->bTriggerWhenPaused = true;
	ReplayAction->ValueType = EInputActionValueType::Boolean;
	ToggleAbsAction->ValueType = EInputActionValueType::Boolean;
	ToggleTractionAction->ValueType = EInputActionValueType::Boolean;
	ErsStrategyAction->ValueType = EInputActionValueType::Boolean;

	SetActiveCamera(0);
}

void AApexFormulaCar::BeginPlay()
{
	Super::BeginPlay();
	const UApexCarTuningDataAsset* Tuning = ResolveTuning();
	Chassis->SetMassOverrideInKg(NAME_None, Tuning->MassKg, true);
	Chassis->SetCenterOfMass(Tuning->CenterOfMassOffsetCm);
	EngineRpm = Tuning->IdleRpm;
	Chassis->OnComponentHit.AddDynamic(this, &AApexFormulaCar::HandleChassisHit);
	ConfigureInputActions();
	EngineLowAudio->SetSound(LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Engine/f1_engine_low.f1_engine_low")));
	EngineMidAudio->SetSound(LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Engine/f1_engine_mid.f1_engine_mid")));
	EngineHighAudio->SetSound(LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Engine/f1_engine_high.f1_engine_high")));
	for (UAudioComponent* Audio : {EngineLowAudio.Get(), EngineMidAudio.Get(), EngineHighAudio.Get()})
	{
		if (Audio != nullptr && Audio->GetSound() != nullptr) Audio->Play();
	}
	UpdateWheelVisuals();
	UE_LOG(LogTemp, Display, TEXT("APEX Formula Car initialized: %.0f kg, %.0f hp-equivalent ICE target, %d forward gears."), Tuning->MassKg, Tuning->PeakPowerWatts / 745.7f, Tuning->GearRatios.Num());
}

void AApexFormulaCar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FVector WorldLocation = GetActorLocation();
	if (WorldLocation.ContainsNaN() || WorldLocation.Z < -1500.0f || WorldLocation.Z > 8000.0f)
	{
		ResetVehicle();
		return;
	}
	if (bReplayPlaying)
	{
		UpdateReplay(DeltaSeconds);
		return;
	}
	ApplyVehicleForces(DeltaSeconds);
	UpdateWheelVisuals();
	UpdateTelemetry(DeltaSeconds);
	UpdateEngineAudio();
	UpdateVehicleCondition(DeltaSeconds);
	const float TrackWetness = Track.IsValid() ? Track->GetWetness() : 0.0f;
	const bool bNeedsSpray = SpeedMps > 8.0f && (TrackWetness > 0.08f || CurrentSurface == EApexSurface::Grass || CurrentSurface == EApexSurface::Gravel);
	if (bNeedsSpray && !WheelSprayFx->IsActive()) WheelSprayFx->Activate();
	else if (!bNeedsSpray && WheelSprayFx->IsActive()) WheelSprayFx->Deactivate();
	if (BrakeInput > 0.88f && SpeedMps > 28.0f && TrackWetness < 0.2f && !TyreBurstFx->IsActive()) TyreBurstFx->Activate(true);
	RecordReplay(DeltaSeconds);
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
		EnhancedInput->BindAction(PhotoModeAction, ETriggerEvent::Started, this, &AApexFormulaCar::TogglePhotoMode);
		EnhancedInput->BindAction(ReplayAction, ETriggerEvent::Started, this, &AApexFormulaCar::TriggerReplay);
		EnhancedInput->BindAction(ToggleAbsAction, ETriggerEvent::Started, this, &AApexFormulaCar::ToggleAbsInput);
		EnhancedInput->BindAction(ToggleTractionAction, ETriggerEvent::Started, this, &AApexFormulaCar::ToggleTractionInput);
		EnhancedInput->BindAction(ErsStrategyAction, ETriggerEvent::Started, this, &AApexFormulaCar::CycleErsStrategyInput);
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

void AApexFormulaCar::SetRaceEnabled(bool bEnabled)
{
	bRaceEnabled = bEnabled;
	if (!bRaceEnabled)
	{
		ThrottleInput = 0.0f;
		BrakeInput = 1.0f;
		bDrsInput = false;
		bErsInput = false;
	}
}

void AApexFormulaCar::SetAiControl(float InThrottle, float InBrake, float InSteering, bool bInDrs, bool bInErs)
{
	bAiControlled = true;
	ThrottleInput = bRaceEnabled ? FMath::Clamp(InThrottle, 0.0f, 1.0f) : 0.0f;
	BrakeInput = bRaceEnabled ? FMath::Clamp(InBrake, 0.0f, 1.0f) : 1.0f;
	SteeringInput = FMath::Clamp(InSteering, -1.0f, 1.0f);
	bDrsInput = bRaceEnabled && bInDrs;
	bErsInput = bRaceEnabled && bInErs;
}

void AApexFormulaCar::SetDriverIdentity(const FString& InName, int32 InNumber)
{
	DriverName = InName;
	CarNumber = InNumber;
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

void AApexFormulaCar::ResetToTransform(const FTransform& Transform)
{
	Chassis->SetSimulatePhysics(false);
	SetActorTransform(Transform, false, nullptr, ETeleportType::TeleportPhysics);
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
	DrivingContext->MapKey(PhotoModeAction, EKeys::P);
	DrivingContext->MapKey(ReplayAction, EKeys::F8);
	DrivingContext->MapKey(ToggleAbsAction, EKeys::B);
	DrivingContext->MapKey(ToggleTractionAction, EKeys::T);
	DrivingContext->MapKey(ErsStrategyAction, EKeys::M);
	DrivingContext->MapKey(ThrottleAction, EKeys::Gamepad_RightTriggerAxis);
	DrivingContext->MapKey(BrakeAction, EKeys::Gamepad_LeftTriggerAxis);
	DrivingContext->MapKey(SteeringAction, EKeys::Gamepad_LeftX);
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
		const bool bAutoDeploy = ErsStrategy == 1 && ThrottleInput > 0.82f && SpeedMps > 25.0f;
		const bool bDeployErs = ErsStrategy != 2 && (bErsInput || bAutoDeploy);
		if (bDeployErs && EnergyMj > 0.0f)
		{
			const float DeployWatts = FMath::Min(120000.0f, EnergyMj * 1000000.0f / DeltaSeconds);
			AvailableDriveForceN += FMath::Min(9500.0f, DeployWatts / FMath::Max(18.0f, FMath::Abs(ForwardSpeedMps)));
			EnergyMj = FMath::Max(0.0f, EnergyMj - DeployWatts * DeltaSeconds / 1000000.0f);
		}
	}

	const float AbsMultiplier = bAbsEnabled ? FMath::Clamp(SpeedMps / 4.0f, 0.3f, 1.0f) : 1.0f;
	const float TotalBrakeForceN = BrakeInput * Tuning->MaxBrakeForceN * AbsMultiplier;
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
		const float TemperatureGrip = 1.0f - FMath::Clamp(FMath::Abs(TyreTemperatureC - 96.0f) / 90.0f, 0.0f, 0.22f);
		const float ConditionGrip = TemperatureGrip * FMath::Lerp(1.0f, 0.78f, TyreWear) * FMath::Lerp(1.0f, 0.7f, Damage);
		const float ConditionedFrictionLimitN = FrictionLimitN * ConditionGrip;
		if (bTractionControlEnabled && !Wheel.bFront && LongitudinalForceN > 0.0f)
		{
			const float TractionReserveN = FMath::Sqrt(FMath::Max(0.0f, FMath::Square(ConditionedFrictionLimitN) - FMath::Square(LateralForceN)));
			LongitudinalForceN = FMath::Min(LongitudinalForceN, TractionReserveN * 0.94f);
		}
		const float CombinedForceN = FMath::Sqrt(FMath::Square(LongitudinalForceN) + FMath::Square(LateralForceN));
		if (CombinedForceN > ConditionedFrictionLimitN && CombinedForceN > KINDA_SMALL_NUMBER)
		{
			const float Scale = ConditionedFrictionLimitN / CombinedForceN;
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
		const float HarvestMultiplier = ErsStrategy == 2 ? 1.55f : 1.0f;
		EnergyMj = FMath::Min(4.0f, EnergyMj + 70000.0f * HarvestMultiplier * BrakeInput * DeltaSeconds / 1000000.0f);
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

void AApexFormulaCar::UpdateEngineAudio()
{
	const float RpmFraction = FMath::Clamp((EngineRpm - 4500.0f) / 10500.0f, 0.0f, 1.0f);
	const float Load = FMath::Max(ThrottleInput, 0.18f);
	if (EngineLowAudio) { EngineLowAudio->SetVolumeMultiplier((1.0f - RpmFraction) * Load); EngineLowAudio->SetPitchMultiplier(FMath::Lerp(0.78f, 1.28f, RpmFraction)); }
	if (EngineMidAudio) { EngineMidAudio->SetVolumeMultiplier((1.0f - FMath::Abs(RpmFraction - .5f) * 2.0f) * Load); EngineMidAudio->SetPitchMultiplier(FMath::Lerp(0.75f, 1.32f, RpmFraction)); }
	if (EngineHighAudio) { EngineHighAudio->SetVolumeMultiplier(RpmFraction * Load); EngineHighAudio->SetPitchMultiplier(FMath::Lerp(0.72f, 1.36f, RpmFraction)); }
}

void AApexFormulaCar::UpdateVehicleCondition(float DeltaSeconds)
{
	const float SpeedFactor = FMath::Clamp(SpeedMps / 90.0f, 0.0f, 1.5f);
	const float Work = FMath::Clamp(ThrottleInput + BrakeInput * 1.35f + FMath::Abs(SteeringInput) * SpeedFactor, 0.0f, 2.5f);
	const float SurfaceHeat = CurrentSurface == EApexSurface::Asphalt ? 0.0f : 7.0f;
	const float TargetTemperature = 72.0f + Work * 24.0f + SurfaceHeat;
	TyreTemperatureC = FMath::FInterpTo(TyreTemperatureC, TargetTemperature, DeltaSeconds, 0.18f);
	TyreWear = FMath::Clamp(TyreWear + Work * SpeedFactor * DeltaSeconds * 0.000018f, 0.0f, 1.0f);
	FuelKg = FMath::Max(0.0f, FuelKg - ThrottleInput * FMath::Lerp(0.00018f, 0.00072f, SpeedFactor) * DeltaSeconds * 60.0f);
}

void AApexFormulaCar::HandleChassisHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	const float ImpactStrength = NormalImpulse.Size() / 100000.0f;
	if (ImpactStrength > 2.0f)
	{
		Damage = FMath::Clamp(Damage + (ImpactStrength - 2.0f) * 0.004f, 0.0f, 1.0f);
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
	if (!bAiControlled)
	{
		ThrottleInput = bRaceEnabled ? FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f) : 0.0f;
	}
}

void AApexFormulaCar::SetBrake(const FInputActionValue& Value)
{
	if (!bAiControlled)
	{
		BrakeInput = bRaceEnabled ? FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f) : 1.0f;
	}
}

void AApexFormulaCar::SetSteering(const FInputActionValue& Value)
{
	if (!bAiControlled)
	{
		SteeringInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
	}
}

void AApexFormulaCar::SetDrs(const FInputActionValue& Value)
{
	if (!bAiControlled)
	{
		bDrsInput = bRaceEnabled && Value.Get<bool>();
	}
}

void AApexFormulaCar::SetErs(const FInputActionValue& Value)
{
	if (!bAiControlled)
	{
		bErsInput = bRaceEnabled && Value.Get<bool>();
	}
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
	if (RaceDirector.IsValid())
	{
		RaceDirector->TogglePause();
	}
}

void AApexFormulaCar::TogglePhotoMode(const FInputActionValue& Value)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		bPhotoMode = !bPhotoMode;
		PlayerController->SetPause(bPhotoMode);
		PlayerController->bShowMouseCursor = bPhotoMode;
	}
}

void AApexFormulaCar::TriggerReplay(const FInputActionValue& Value)
{
	if (bAiControlled || bReplayPlaying || ReplayFrames.Num() < 90)
	{
		return;
	}
	bReplayPlaying = true;
	ReplayPlaybackIndex = FMath::Max(0, ReplayFrames.Num() - 240);
	ReplayPlaybackAccumulator = 0.0f;
	Chassis->SetSimulatePhysics(false);
	SetActiveCamera(0);
}

void AApexFormulaCar::RecordReplay(float DeltaSeconds)
{
	if (bAiControlled || bPhotoMode)
	{
		return;
	}
	ReplayRecordAccumulator += DeltaSeconds;
	if (ReplayRecordAccumulator < 1.0f / 30.0f)
	{
		return;
	}
	ReplayRecordAccumulator = 0.0f;
	ReplayFrames.Add(GetActorTransform());
	if (ReplayFrames.Num() > 900)
	{
		ReplayFrames.RemoveAt(0, ReplayFrames.Num() - 900, EAllowShrinking::No);
	}
}

void AApexFormulaCar::UpdateReplay(float DeltaSeconds)
{
	ReplayPlaybackAccumulator += DeltaSeconds;
	while (ReplayPlaybackAccumulator >= 1.0f / 30.0f && ReplayPlaybackIndex < ReplayFrames.Num())
	{
		ReplayPlaybackAccumulator -= 1.0f / 30.0f;
		SetActorTransform(ReplayFrames[ReplayPlaybackIndex++], false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (ReplayPlaybackIndex >= ReplayFrames.Num())
	{
		bReplayPlaying = false;
		Chassis->SetSimulatePhysics(true);
	}
}

FString AApexFormulaCar::GetErsStrategyName() const
{
	static const TCHAR* Names[] = {TEXT("BALANCED"), TEXT("OVERTAKE"), TEXT("HARVEST")};
	return Names[FMath::Clamp(ErsStrategy, 0, 2)];
}

void AApexFormulaCar::ToggleAbsAssist()
{
	bAbsEnabled = !bAbsEnabled;
}

void AApexFormulaCar::ToggleTractionAssist()
{
	bTractionControlEnabled = !bTractionControlEnabled;
}

void AApexFormulaCar::CycleErsStrategy()
{
	ErsStrategy = (ErsStrategy + 1) % 3;
}

void AApexFormulaCar::ToggleAbsInput(const FInputActionValue& Value)
{
	ToggleAbsAssist();
}

void AApexFormulaCar::ToggleTractionInput(const FInputActionValue& Value)
{
	ToggleTractionAssist();
}

void AApexFormulaCar::CycleErsStrategyInput(const FInputActionValue& Value)
{
	CycleErsStrategy();
}
