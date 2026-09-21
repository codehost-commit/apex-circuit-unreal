#pragma once

#include "GameFramework/GameModeBase.h"
#include "ApexCircuitGameMode.generated.h"

class AApexFormulaCar;
class AApexRaceDirector;
class AApexTrackActor;

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

private:
	void SpawnPlayerCar();

	UPROPERTY(Transient)
	TObjectPtr<AApexTrackActor> Track;

	UPROPERTY(Transient)
	TObjectPtr<AApexFormulaCar> PlayerCar;

	UPROPERTY(Transient)
	TObjectPtr<AApexRaceDirector> RaceDirector;
};
