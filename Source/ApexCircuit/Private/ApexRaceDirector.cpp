#include "ApexRaceDirector.h"

#include "ApexFormulaCar.h"
#include "ApexTrackActor.h"

AApexRaceDirector::AApexRaceDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	SetActorHiddenInGame(true);
}

void AApexRaceDirector::Initialize(AApexTrackActor* InTrack, AApexFormulaCar* InPlayerCar)
{
	Track = InTrack;
	PlayerCar = InPlayerCar;
	bHasPreviousProgress = false;
	bDrsArmed = false;
	bWasInDrsZone = false;
	NextCheckpointIndex = 0;
	CurrentLap = 1;
	CurrentSector = 1;
	SessionTimeSeconds = 0.0f;
	CurrentLapTimeSeconds = 0.0f;
	LastLapTimeSeconds = 0.0f;
	UE_LOG(LogTemp, Display, TEXT("APEX Race Director initialized: five ordered checkpoints, three sectors, and %d DRS zones."), Track.IsValid() ? Track->GetDrsZones().Num() : 0);
}

void AApexRaceDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateRaceState(DeltaSeconds);
}

void AApexRaceDirector::UpdateRaceState(float DeltaSeconds)
{
	if (!Track.IsValid() || !PlayerCar.IsValid())
	{
		return;
	}

	const float TrackLengthCm = Track->GetTrackLengthCm();
	if (TrackLengthCm <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	SessionTimeSeconds += DeltaSeconds;
	CurrentLapTimeSeconds += DeltaSeconds;
	CurrentProgressCm = Track->GetProgressAtWorldLocation(PlayerCar->GetActorLocation());
	CurrentSector = Track->GetSectorForDistance(CurrentProgressCm);

	if (!bHasPreviousProgress)
	{
		PreviousProgressCm = CurrentProgressCm;
		bHasPreviousProgress = true;
		PlayerCar->SetSafeProgressCm(CurrentProgressCm);
		return;
	}

	const float TravelCm = ForwardDistance(PreviousProgressCm, CurrentProgressCm, TrackLengthCm);
	const bool bPlausibleForwardTravel = TravelCm > 0.0f && TravelCm < TrackLengthCm * 0.25f;
	if (bPlausibleForwardTravel)
	{
		const TArray<float>& Checkpoints = Track->GetCheckpointDistancesCm();
		while (Checkpoints.IsValidIndex(NextCheckpointIndex) && CrossedForwardDistance(PreviousProgressCm, CurrentProgressCm, Checkpoints[NextCheckpointIndex], TrackLengthCm))
		{
			PlayerCar->SetSafeProgressCm(Checkpoints[NextCheckpointIndex]);
			++NextCheckpointIndex;
		}

		for (const FApexDrsZone& Zone : Track->GetDrsZones())
		{
			if (CrossedForwardDistance(PreviousProgressCm, CurrentProgressCm, Zone.DetectionDistanceCm, TrackLengthCm))
			{
				bDrsArmed = true;
			}
		}

		if (PreviousProgressCm > TrackLengthCm * 0.85f && CurrentProgressCm < TrackLengthCm * 0.15f)
		{
			if (NextCheckpointIndex == Checkpoints.Num())
			{
				LastLapTimeSeconds = CurrentLapTimeSeconds;
				CurrentLapTimeSeconds = 0.0f;
				++CurrentLap;
			}
			NextCheckpointIndex = 0;
			PlayerCar->SetSafeProgressCm(0.0f);
		}
	}

	const bool bInDrsZone = Track->IsDistanceInDrsZone(CurrentProgressCm);
	PlayerCar->SetDrsAvailability(bDrsArmed && bInDrsZone);
	if (bWasInDrsZone && !bInDrsZone)
	{
		bDrsArmed = false;
	}
	bWasInDrsZone = bInDrsZone;

	PreviousProgressCm = CurrentProgressCm;
}

float AApexRaceDirector::ForwardDistance(float FromCm, float ToCm, float TrackLengthCm)
{
	float Delta = ToCm - FromCm;
	if (Delta < -TrackLengthCm * 0.5f)
	{
		Delta += TrackLengthCm;
	}
	else if (Delta > TrackLengthCm * 0.5f)
	{
		Delta -= TrackLengthCm;
	}
	return Delta;
}

bool AApexRaceDirector::CrossedForwardDistance(float FromCm, float ToCm, float TargetCm, float TrackLengthCm)
{
	const float TravelCm = ForwardDistance(FromCm, ToCm, TrackLengthCm);
	if (TravelCm <= 0.0f)
	{
		return false;
	}

	float TargetDeltaCm = TargetCm - FromCm;
	if (TargetDeltaCm < 0.0f)
	{
		TargetDeltaCm += TrackLengthCm;
	}
	return TargetDeltaCm <= TravelCm;
}
