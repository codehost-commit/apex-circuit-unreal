#pragma once

#include "GameFramework/GameModeBase.h"
#include "ApexCircuitGameMode.generated.h"

class AApexFormulaCar;
class AApexRaceDirector;
class AApexTrackActor;
class AApexPresentationDirector;

UCLASS()
class APEXCIRCUIT_API AApexCircuitGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AApexCircuitGameMode();

	virtual void BeginPlay() override;

	AApexTrackActor* GetTrack() const { return Track; }
	AApexFormulaCar* GetPlayerCar() const { return PlayerCar; }
	AApexRaceDirector* GetRaceDirector() const { return RaceDirector; }
	AApexPresentationDirector* GetPresentationDirector() const { return PresentationDirector; }

private:
	void SpawnPlayerCar();
	void SpawnOpponentField();

	UPROPERTY(Transient)
	TObjectPtr<AApexTrackActor> Track;

	UPROPERTY(Transient)
	TObjectPtr<AApexFormulaCar> PlayerCar;

	UPROPERTY(Transient)
	TObjectPtr<AApexRaceDirector> RaceDirector;

	UPROPERTY(Transient)
	TObjectPtr<AApexPresentationDirector> PresentationDirector;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AApexFormulaCar>> OpponentCars;
};
