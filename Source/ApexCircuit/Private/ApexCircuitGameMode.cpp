#include "ApexCircuitGameMode.h"

#include "ApexDebugHUD.h"
#include "ApexFormulaCar.h"
#include "ApexRaceDirector.h"
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
	UE_LOG(LogTemp, Display, TEXT("APEX Phase 1 ready: player car spawned on the original circuit start/finish seam."));
}
