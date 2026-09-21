#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApexPresentationDirector.generated.h"

class AApexFormulaCar;
class AApexTrackActor;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UMaterialParameterCollection;
class UPostProcessComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class USpotLightComponent;
class UVolumetricCloudComponent;

/**
 * Runtime-authored Phase 3 world presentation. Keeping the authored layout in C++
 * makes the showcase environment deterministic and rebuildable on every machine.
 */
UCLASS()
class APEXCIRCUIT_API AApexPresentationDirector : public AActor
{
	GENERATED_BODY()

public:
	AApexPresentationDirector();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void Initialize(AApexTrackActor* InTrack, AApexFormulaCar* InPlayerCar);

	UFUNCTION(BlueprintCallable, Category = "Apex|Presentation")
	void SetRainIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Apex|Presentation")
	void SetTimeOfDay(float Hour);

	UFUNCTION(BlueprintPure, Category = "Apex|Presentation")
	float GetRainIntensity() const { return RainIntensity; }

	UFUNCTION(BlueprintPure, Category = "Apex|Presentation")
	float GetTimeOfDay() const { return TimeOfDayHours; }

private:
	void BuildTracksideEnvironment();
	void BuildFloodlights();
	void BuildRainField();
	void UpdateRain(float DeltaSeconds);
	void ApplyLighting();
	void AddTracksideInstance(UHierarchicalInstancedStaticMeshComponent* Component, float DistanceCm, float LateralCm, const FVector& Scale, float HeightCm = 0.0f, float YawOffset = 0.0f);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Lighting")
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Lighting")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Lighting")
	TObjectPtr<USkyAtmosphereComponent> Atmosphere;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Lighting")
	TObjectPtr<UExponentialHeightFogComponent> Fog;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Lighting")
	TObjectPtr<UVolumetricCloudComponent> Clouds;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Lighting")
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Armco;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> FencePosts;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LightPoles;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LightHeads;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Grandstands;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Buildings;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> SponsorBoards;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PitWall;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> MarshalPosts;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrackCameras;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Vegetation;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> BroadleafTrees;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Scenery")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PineTrees;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Weather")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RainStreaks;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Weather")
	TObjectPtr<UMaterialParameterCollection> WeatherCollection;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USpotLightComponent>> Floodlights;

	TWeakObjectPtr<AApexTrackActor> Track;
	TWeakObjectPtr<AApexFormulaCar> PlayerCar;
	TArray<FVector> RainOffsets;
	float RainIntensity = 0.18f;
	float TimeOfDayHours = 17.25f;
	bool bEnvironmentBuilt = false;
};
