#include "ApexCircuitGameMode.h"

#include "ApexDebugHUD.h"
#include "ApexAiDriverComponent.h"
#include "ApexFormulaCar.h"
#include "ApexRaceDirector.h"
#include "ApexRaceHUDWidget.h"
#include "ApexTrackActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AApexCircuitGameMode::AApexCircuitGameMode()
{
	HUDClass = AApexDebugHUD::StaticClass();
	bStartPlayersAsSpectators = true;
}

void AApexCircuitGameMode::BeginPlay()
{
	Super::BeginPlay();

	Track = Cast<AApexTrackActor>(UGameplayStatics::GetActorOfClass(this, AApexTrackActor::StaticClass()));
	if (Track == nullptr)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Track = GetWorld()->SpawnActor<AApexTrackActor>(AApexTrackActor::StaticClass(), FTransform::Identity, SpawnParameters);
	}
	if (Track != nullptr)
	{
		Track->BuildTrack();
	}

	GetWorldTimerManager().SetTimerForNextTick(this, &AApexCircuitGameMode::SpawnPlayerCar);
}

void AApexCircuitGameMode::SpawnPlayerCar()
{
	if (Track == nullptr || Track->GetTrackLengthCm() <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController == nullptr)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AApexCircuitGameMode::SpawnPlayerCar);
		return;
	}

	if (APawn* ExistingPawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		ExistingPawn->Destroy();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	PlayerCar = GetWorld()->SpawnActor<AApexFormulaCar>(AApexFormulaCar::StaticClass(), Track->GetSpawnTransformAtDistance(0.0f), SpawnParameters);
	if (PlayerCar == nullptr)
	{
		return;
	}

	PlayerCar->SetTrack(Track);
	PlayerController->Possess(PlayerCar);
	PlayerController->SetControlRotation(PlayerCar->GetActorRotation());

	RaceDirector = GetWorld()->SpawnActor<AApexRaceDirector>(AApexRaceDirector::StaticClass(), FTransform::Identity, SpawnParameters);
	if (RaceDirector != nullptr)
	{
		RaceDirector->Initialize(Track, PlayerCar);
		PlayerCar->SetRaceDirector(RaceDirector);
	}
	SpawnOpponentField();
	if (APlayerController* Controller = GetWorld()->GetFirstPlayerController())
	{
		UApexRaceHUDWidget* RaceHud = CreateWidget<UApexRaceHUDWidget>(Controller, UApexRaceHUDWidget::StaticClass());
		if (RaceHud != nullptr)
		{
			RaceHud->AddToViewport(10);
			Controller->bShowMouseCursor = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			Controller->SetInputMode(InputMode);
		}
	}
	UE_LOG(LogTemp, Display, TEXT("APEX Phase 2 ready: player, deterministic opponent field and session director spawned."));
}

void AApexCircuitGameMode::SpawnOpponentField()
{
	if (!Track || !RaceDirector) return;
	TArray<AApexFormulaCar*> Field;
	Field.Add(PlayerCar);
	static const TCHAR* Names[] = {TEXT("M. VEGA"), TEXT("J. PARK"), TEXT("L. SATO"), TEXT("R. KIM"), TEXT("A. MORENO"), TEXT("I. BLAKE"), TEXT("T. RAHMAN")};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Names); ++Index)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const float Distance = FMath::Fmod(Track->GetTrackLengthCm() - 2450.0f - Index * 1100.0f, Track->GetTrackLengthCm());
		AApexFormulaCar* Opponent = GetWorld()->SpawnActor<AApexFormulaCar>(AApexFormulaCar::StaticClass(), Track->GetSpawnTransformAtDistance(Distance, Index % 2 ? 280.0f : -280.0f), Params);
		if (!Opponent) continue;
		Opponent->SetTrack(Track);
		Opponent->SetDriverIdentity(Names[Index], 12 + Index);
		Opponent->SetRaceEnabled(false);
		UApexAiDriverComponent* Driver = NewObject<UApexAiDriverComponent>(Opponent, *FString::Printf(TEXT("ApexAiDriver_%d"), Index));
		Driver->RegisterComponent();
		Driver->Initialize(Track, 0.975f + Index * 0.004f, 2026 + Index);
		OpponentCars.Add(Opponent);
		Field.Add(Opponent);
	}
	RaceDirector->SetField(Field);
}
