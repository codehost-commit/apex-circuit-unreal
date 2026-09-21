#include "ApexRaceDirector.h"

#include "ApexFormulaCar.h"
#include "ApexSessionConfigDataAsset.h"
#include "ApexTrackActor.h"
#include "Kismet/GameplayStatics.h"

AApexRaceDirector::AApexRaceDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	SetActorHiddenInGame(true);
}

void AApexRaceDirector::Initialize(AApexTrackActor* InTrack, AApexFormulaCar* InPlayerCar)
{
	Track = InTrack; PlayerCar = InPlayerCar;
	SessionConfig = LoadObject<UApexSessionConfigDataAsset>(nullptr, TEXT("/Game/Data/DA_ApexSessionConfig.DA_ApexSessionConfig"));
	if (!SessionConfig) SessionConfig = NewObject<UApexSessionConfigDataAsset>(this, TEXT("RuntimeApexSessionConfig"));
	SetField({InPlayerCar});
	InPlayerCar->SetRaceEnabled(false);
	UE_LOG(LogTemp, Display, TEXT("APEX Phase 2 race director ready: sessions, start sequence, timing, DRS and classification."));
}

void AApexRaceDirector::SetField(const TArray<AApexFormulaCar*>& InField)
{
	Field.Reset(); CarStates.Reset();
	for (AApexFormulaCar* Car : InField) if (IsValid(Car)) { Field.Add(Car); CarStates.Add(Car, FApexCarRaceState()); }
	UpdateClassification();
}

void AApexRaceDirector::StartSession(EApexSessionMode InMode)
{
	if (!Track || !PlayerCar || InMode == EApexSessionMode::Menu || InMode == EApexSessionMode::Results) return;
	SessionMode = InMode; SessionTimeSeconds = 0.0f; StartLights = 0; StartTimerSeconds = 0.0f;
	SessionLapTarget = InMode == EApexSessionMode::GPWS ? SessionConfig->GpwsRaceLaps : SessionConfig->RaceLaps;
	ResetFieldForSession();
	const bool bGrid = InMode == EApexSessionMode::Race || InMode == EApexSessionMode::GPWS;
	SessionState = bGrid ? EApexSessionState::Grid : EApexSessionState::Green;
	for (AApexFormulaCar* Car : Field) if (IsValid(Car)) Car->SetRaceEnabled(!bGrid);
	SetMessage(bGrid ? TEXT("FORMING GRID") : (InMode == EApexSessionMode::Qualifying ? TEXT("QUALIFYING OPEN") : TEXT("SESSION LIVE")));
}

void AApexRaceDirector::RestartSession() { StartSession(SessionMode == EApexSessionMode::Menu ? EApexSessionMode::Practice : SessionMode); }

void AApexRaceDirector::TogglePause()
{
	if (SessionState == EApexSessionState::Menu || SessionState == EApexSessionState::Finished) return;
	const bool bPause = SessionState != EApexSessionState::Paused;
	UGameplayStatics::SetGamePaused(this, bPause);
	SessionState = bPause ? EApexSessionState::Paused : EApexSessionState::Green;
	SetMessage(bPause ? TEXT("PAUSED") : TEXT("RACE RESUMED"));
}

void AApexRaceDirector::RecoverPlayer()
{
	if (!PlayerCar || !Track || SessionState == EApexSessionState::Menu) return;
	FApexCarRaceState& State = StateFor(PlayerCar);
	PlayerCar->ResetToTransform(Track->GetSpawnTransformAtDistance(State.SafeProgressCm));
	State.bLapValid = false;
	if (SessionMode == EApexSessionMode::Race) State.PenaltySeconds += SessionConfig->RecoveryPenaltySeconds;
	SetMessage(TEXT("RECOVERED - LAP INVALID"));
}

void AApexRaceDirector::ResetFieldForSession()
{
	CarStates.Reset();
	for (int32 Index = 0; Index < Field.Num(); ++Index)
	{
		AApexFormulaCar* Car = Field[Index]; if (!IsValid(Car)) continue;
		const bool bSpread = SessionMode == EApexSessionMode::Qualifying || SessionMode == EApexSessionMode::TimeTrial || SessionMode == EApexSessionMode::Practice;
		const float Grid = FMath::Fmod(Track->GetTrackLengthCm() - 1600.0f - (Index / 2) * 850.0f, Track->GetTrackLengthCm());
		const float Distance = bSpread && Index > 0 ? FMath::Fmod(Grid - Index * 28000.0f + Track->GetTrackLengthCm(), Track->GetTrackLengthCm()) : Grid;
		Car->ResetToTransform(Track->GetSpawnTransformAtDistance(Distance, bSpread ? 0.0f : (Index % 2 == 0 ? -280.0f : 280.0f)));
		Car->SetSafeProgressCm(Distance); Car->SetDrsAvailability(false);
		FApexCarRaceState State; State.PreviousProgressCm = Distance; State.SafeProgressCm = Distance; CarStates.Add(Car, State);
	}
	UpdateClassification();
}

void AApexRaceDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Track || !PlayerCar || SessionState == EApexSessionState::Menu || SessionState == EApexSessionState::Paused || SessionState == EApexSessionState::Finished) return;
	if (SessionState == EApexSessionState::Grid || SessionState == EApexSessionState::Lights) { UpdateStartSequence(DeltaSeconds); return; }
	SessionTimeSeconds += DeltaSeconds;
	for (AApexFormulaCar* Car : Field) if (IsValid(Car)) UpdateCarProgress(Car, DeltaSeconds);
	UpdateClassification();
	if (SessionMode == EApexSessionMode::Qualifying && SessionTimeSeconds >= SessionConfig->QualifyingDurationSeconds) { SessionState = EApexSessionState::Finished; SessionMode = EApexSessionMode::Results; SetMessage(TEXT("QUALIFYING COMPLETE")); }
}

void AApexRaceDirector::UpdateStartSequence(float DeltaSeconds)
{
	StartTimerSeconds += DeltaSeconds;
	if (SessionState == EApexSessionState::Grid && StartTimerSeconds >= 1.5f) { SessionState = EApexSessionState::Lights; StartTimerSeconds = 0.0f; NextLightSeconds = SessionConfig->LightStepSeconds; SetMessage(TEXT("START SEQUENCE")); return; }
	if (SessionState != EApexSessionState::Lights) return;
	if (StartLights < 5 && StartTimerSeconds >= NextLightSeconds) { ++StartLights; StartTimerSeconds = 0.0f; NextLightSeconds = StartLights == 5 ? Random.FRandRange(SessionConfig->MinimumLightsOutSeconds, SessionConfig->MaximumLightsOutSeconds) : SessionConfig->LightStepSeconds; return; }
	if (StartLights == 5 && StartTimerSeconds >= NextLightSeconds) { StartLights = 0; SessionState = EApexSessionState::Green; for (AApexFormulaCar* Car : Field) if (IsValid(Car)) Car->SetRaceEnabled(true); SetMessage(TEXT("LIGHTS OUT")); }
}

void AApexRaceDirector::UpdateCarProgress(AApexFormulaCar* Car, float DeltaSeconds)
{
	FApexCarRaceState& State = StateFor(Car); const float Length = Track->GetTrackLengthCm(); const float Progress = Track->GetProgressAtWorldLocation(Car->GetActorLocation());
	if (!State.bInitialized) { State.PreviousProgressCm = Progress; State.bInitialized = true; return; }
	const float Travel = ForwardDistance(State.PreviousProgressCm, Progress, Length); State.CurrentLapSeconds += DeltaSeconds;
	const FApexSurfaceSample Surface = Track->SampleSurfaceAtWorldLocation(Car->GetActorLocation());
	State.OffTrackSeconds = FMath::Abs(Surface.LateralOffsetCm) > 1780.0f ? State.OffTrackSeconds + DeltaSeconds : 0.0f;
	if (State.OffTrackSeconds > 0.4f) State.bLapValid = false;
	if (Travel > 0.0f && Travel < Length * 0.12f)
	{
		const TArray<float>& Checks = Track->GetCheckpointDistancesCm();
		while (Checks.IsValidIndex(State.NextCheckpoint) && CrossedForwardDistance(State.PreviousProgressCm, Progress, Checks[State.NextCheckpoint], Length)) { State.SafeProgressCm = Checks[State.NextCheckpoint++]; State.Sector = Track->GetSectorForDistance(Progress); }
		if (State.PreviousProgressCm > Length * .85f && Progress < Length * .15f) { if (State.NextCheckpoint == Checks.Num()) { State.LastLapSeconds = State.CurrentLapSeconds; if (State.bLapValid) State.BestLapSeconds = FMath::Min(State.BestLapSeconds, State.LastLapSeconds); ++State.Lap; State.CurrentLapSeconds = 0.0f; State.bLapValid = true; if ((SessionMode == EApexSessionMode::Race || SessionMode == EApexSessionMode::GPWS) && State.Lap > SessionLapTarget) { State.bFinished = true; Car->SetRaceEnabled(false); } } State.NextCheckpoint = 0; }
	}
	UpdateDrsForCar(Car, State); State.PreviousProgressCm = Progress; if (Car == PlayerCar) CurrentProgressCm = Progress;
}

void AApexRaceDirector::UpdateDrsForCar(AApexFormulaCar* Car, const FApexCarRaceState& State)
{
	bool bEligible = SessionMode != EApexSessionMode::Race && SessionMode != EApexSessionMode::GPWS;
	if (!bEligible) for (AApexFormulaCar* Other : Field) if (Other != Car && IsValid(Other)) { const float Gap = ForwardDistance(State.PreviousProgressCm, StateFor(Other).PreviousProgressCm, Track->GetTrackLengthCm()); if (Gap > 0.0f && Gap / FMath::Max(Car->GetSpeedMps() * 100.0f, 1500.0f) <= SessionConfig->DrsGapSeconds) { bEligible = true; break; } }
	Car->SetDrsAvailability(bEligible && Track->IsDistanceInDrsZone(State.PreviousProgressCm));
}

void AApexRaceDirector::UpdateClassification()
{
	Classification.Reset(); if (!Track) return;
	struct FEntry { AApexFormulaCar* Car; float Progress; }; TArray<FEntry> Sorted;
	for (AApexFormulaCar* Car : Field) if (IsValid(Car)) { const FApexCarRaceState& State = StateFor(Car); Sorted.Add({Car, State.Lap * Track->GetTrackLengthCm() + State.PreviousProgressCm}); }
	Sorted.Sort([](const FEntry& A, const FEntry& B) { return A.Progress > B.Progress; });
	for (int32 Index = 0; Index < Sorted.Num(); ++Index) { const FApexCarRaceState& State = StateFor(Sorted[Index].Car); FApexClassificationRow Row; Row.Position = Index + 1; Row.DriverName = Sorted[Index].Car->GetDriverName(); Row.CarNumber = Sorted[Index].Car->GetCarNumber(); Row.CompletedLaps = State.Lap - 1; Row.BestLapSeconds = FMath::IsFinite(State.BestLapSeconds) ? State.BestLapSeconds : 0.0f; Row.PenaltySeconds = State.PenaltySeconds; Row.bFinished = State.bFinished; Row.GapSeconds = Index ? (Sorted[0].Progress - Sorted[Index].Progress) / FMath::Max(Sorted[Index].Car->GetSpeedMps() * 100.0f, 2000.0f) : 0.0f; Classification.Add(Row); }
}

FApexCarRaceState& AApexRaceDirector::StateFor(AApexFormulaCar* Car) { return CarStates.FindOrAdd(Car); }
int32 AApexRaceDirector::GetCurrentLap() const { return PlayerCar ? CarStates.FindRef(PlayerCar).Lap : 1; }
int32 AApexRaceDirector::GetCurrentSector() const { return PlayerCar ? CarStates.FindRef(PlayerCar).Sector : 1; }
float AApexRaceDirector::GetCurrentLapTimeSeconds() const { return PlayerCar ? CarStates.FindRef(PlayerCar).CurrentLapSeconds : 0.0f; }
float AApexRaceDirector::GetLastLapTimeSeconds() const { return PlayerCar ? CarStates.FindRef(PlayerCar).LastLapSeconds : 0.0f; }
bool AApexRaceDirector::IsDrsEligible() const { return PlayerCar && PlayerCar->IsDrsAvailable(); }
bool AApexRaceDirector::IsCurrentLapValid() const { return PlayerCar && CarStates.FindRef(PlayerCar).bLapValid; }
void AApexRaceDirector::SetMessage(const FString& InMessage) { RaceMessage = InMessage; }
float AApexRaceDirector::ForwardDistance(float FromCm, float ToCm, float TrackLengthCm) { float Delta = ToCm - FromCm; if (Delta < -TrackLengthCm * .5f) Delta += TrackLengthCm; else if (Delta > TrackLengthCm * .5f) Delta -= TrackLengthCm; return Delta; }
bool AApexRaceDirector::CrossedForwardDistance(float FromCm, float ToCm, float TargetCm, float TrackLengthCm) { const float Travel = ForwardDistance(FromCm, ToCm, TrackLengthCm); if (Travel <= 0.0f) return false; float Delta = TargetCm - FromCm; if (Delta < 0.0f) Delta += TrackLengthCm; return Delta <= Travel; }
