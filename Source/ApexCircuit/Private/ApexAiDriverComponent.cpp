#include "ApexAiDriverComponent.h"

#include "ApexFormulaCar.h"
#include "ApexTrackActor.h"
#include "Components/SplineComponent.h"

UApexAiDriverComponent::UApexAiDriverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UApexAiDriverComponent::BeginPlay()
{
	Super::BeginPlay();
	Car = Cast<AApexFormulaCar>(GetOwner());
}

void UApexAiDriverComponent::Initialize(AApexTrackActor* InTrack, float InPaceScale, int32 InSeed)
{
	Track = InTrack;
	PaceScale = FMath::Clamp(InPaceScale, 0.94f, 1.03f);
	Random.Initialize(InSeed);
}

float UApexAiDriverComponent::ComputeTargetSpeed(float ProgressCm) const
{
	if (!Track.IsValid() || Track->GetCenterlineSpline() == nullptr)
	{
		return 45.0f;
	}
	const float Length = Track->GetTrackLengthCm();
	const FVector Here = Track->GetCenterlineSpline()->GetLocationAtDistanceAlongSpline(ProgressCm, ESplineCoordinateSpace::World);
	const FVector Ahead = Track->GetCenterlineSpline()->GetLocationAtDistanceAlongSpline(FMath::Fmod(ProgressCm + 1800.0f, Length), ESplineCoordinateSpace::World);
	const FVector Further = Track->GetCenterlineSpline()->GetLocationAtDistanceAlongSpline(FMath::Fmod(ProgressCm + 3600.0f, Length), ESplineCoordinateSpace::World);
	const FVector A = (Ahead - Here).GetSafeNormal2D();
	const FVector B = (Further - Ahead).GetSafeNormal2D();
	const float Corner = 1.0f - FMath::Clamp(FVector::DotProduct(A, B), -1.0f, 1.0f);
	return FMath::Clamp((83.0f - Corner * 145.0f) * PaceScale, 19.0f, 86.0f);
}

void UApexAiDriverComponent::TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);
	ControlAccumulator += DeltaSeconds;
	if (ControlAccumulator < 1.0f / 30.0f || !Car.IsValid() || !Track.IsValid())
	{
		return;
	}
	const float Step = ControlAccumulator;
	ControlAccumulator = 0.0f;
	if (!Car->IsRaceEnabled())
	{
		Car->SetAiControl(0.0f, 1.0f, 0.0f, false, false);
		return;
	}
	const float Progress = Track->GetProgressAtWorldLocation(Car->GetActorLocation());
	const float LookAheadCm = FMath::Clamp(650.0f + Car->GetSpeedMps() * 43.0f, 650.0f, 3000.0f);
	const FVector Target = Track->GetCenterlineSpline()->GetLocationAtDistanceAlongSpline(FMath::Fmod(Progress + LookAheadCm, Track->GetTrackLengthCm()), ESplineCoordinateSpace::World);
	const FVector LocalTarget = Car->GetActorTransform().InverseTransformPosition(Target);
	const float Steer = FMath::Clamp(FMath::Atan2(LocalTarget.Y, FMath::Max(120.0f, LocalTarget.X)) * 1.65f, -1.0f, 1.0f);
	const float TargetSpeed = ComputeTargetSpeed(Progress + LookAheadCm);
	const float Error = TargetSpeed - Car->GetSpeedMps();
	const float Brake = Error < -1.5f ? FMath::Clamp(-Error / 17.0f, 0.0f, 1.0f) : 0.0f;
	const float Throttle = Brake > 0.0f ? 0.0f : FMath::Clamp(Error / 10.0f + 0.28f, 0.20f, 1.0f);
	const bool bStraight = FMath::Abs(Steer) < 0.16f;
	Car->SetAiControl(Throttle, Brake, Steer, Car->IsDrsAvailable() && bStraight, bStraight && Error > 4.0f);
}
