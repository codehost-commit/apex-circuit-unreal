#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApexCircuitTypes.h"
#include "ApexTrackActor.generated.h"

class UBoxComponent;
class UProceduralMeshComponent;
class USceneComponent;
class USplineComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class APEXCIRCUIT_API AApexTrackActor : public AActor
{
	GENERATED_BODY()

public:
	AApexTrackActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Apex|Track")
	void BuildTrack();

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	float GetTrackLengthCm() const { return TrackLengthCm; }

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	float GetTrackLengthMetres() const { return TrackLengthCm / 100.0f; }

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	float GetProgressAtWorldLocation(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	FApexSurfaceSample SampleSurfaceAtWorldLocation(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	FTransform GetSpawnTransformAtDistance(float DistanceCm, float LateralOffsetCm = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	int32 GetSectorForDistance(float DistanceCm) const;

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	bool IsDistanceInDrsZone(float DistanceCm) const;

	/** Drives the shared wet-surface response for road, kerb, grass and gravel. */
	UFUNCTION(BlueprintCallable, Category = "Apex|Track")
	void SetWetness(float InWetness);

	UFUNCTION(BlueprintPure, Category = "Apex|Track")
	float GetWetness() const { return Wetness; }

	const TArray<float>& GetCheckpointDistancesCm() const { return CheckpointDistancesCm; }
	const TArray<float>& GetSectorDistancesCm() const { return SectorDistancesCm; }
	const TArray<FApexDrsZone>& GetDrsZones() const { return DrsZones; }
	USplineComponent* GetCenterlineSpline() const { return CenterlineSpline; }

private:
	bool LoadLayout();
	void BuildRoundedCenterline();
	void BuildSurfaceMeshes();
	void BuildTimingData();
	FVector SourceToLocal(const FVector2D& SourcePoint) const;
	float CurvatureAtIndex(int32 Index) const;
	bool IsKerbSegment(int32 Index) const;
	bool IsInsideGravel(const FVector2D& WorldPoint) const;
	static bool IsPointInPolygon(const FVector2D& Point, const TArray<FVector2D>& Polygon);
	static float WrapDistance(float Distance, float Length);

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Apex|Track", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> CenterlineSpline;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<UProceduralMeshComponent> RoadMesh;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<UProceduralMeshComponent> KerbMesh;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<UProceduralMeshComponent> GravelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<UBoxComponent> GroundCollision;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<UStaticMeshComponent> GroundVisual;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	TObjectPtr<UStaticMeshComponent> StartFinishVisual;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SurfaceMaterials;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	float TrackLengthCm = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	float RoadHalfWidthCm = 1050.0f;

	UPROPERTY(VisibleAnywhere, Category = "Apex|Track")
	float KerbWidthCm = 725.0f;

	float KerbSurfaceWidthCm = 62.0f;
	float KerbHeightCm = 5.5f;
	float SampleStepCm = 160.0f;
	FVector2D SourceOrigin = FVector2D(1600.0f, 1500.0f);
	float MetresPerSourceUnit = 0.14f;
	bool bTrackBuilt = false;
	float Wetness = 0.0f;

	TArray<FVector2D> SourcePoints;
	TArray<FVector> CenterlinePoints;
	TArray<FVector> Tangents;
	TArray<FVector> LateralNormals;
	TArray<float> CumulativeDistancesCm;
	TArray<bool> KerbSegments;
	TArray<TArray<FVector2D>> GravelPolygons;
	TArray<float> CheckpointFractions;
	TArray<FVector2D> SectorAnchors;
	TArray<TPair<float, float>> DrsFractions;
	TArray<float> CheckpointDistancesCm;
	TArray<float> SectorDistancesCm;
	TArray<FApexDrsZone> DrsZones;
};
