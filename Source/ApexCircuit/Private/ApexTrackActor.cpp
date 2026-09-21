#include "ApexTrackActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProceduralMeshComponent.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	bool ReadPoint(const TSharedPtr<FJsonValue>& Value, FVector2D& OutPoint)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!Value.IsValid() || !Value->TryGetArray(Values) || Values == nullptr || Values->Num() != 2)
		{
			return false;
		}

		OutPoint.X = static_cast<float>((*Values)[0]->AsNumber());
		OutPoint.Y = static_cast<float>((*Values)[1]->AsNumber());
		return true;
	}

	bool ReadPointArray(const TArray<TSharedPtr<FJsonValue>>* Values, TArray<FVector2D>& OutPoints)
	{
		if (Values == nullptr)
		{
			return false;
		}

		OutPoints.Reset();
		for (const TSharedPtr<FJsonValue>& Value : *Values)
		{
			FVector2D Point;
			if (!ReadPoint(Value, Point))
			{
				return false;
			}
			OutPoints.Add(Point);
		}
		return !OutPoints.IsEmpty();
	}
}

AApexTrackActor::AApexTrackActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CenterlineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CenterlineSpline"));
	CenterlineSpline->SetupAttachment(SceneRoot);
	CenterlineSpline->SetClosedLoop(true, false);

	RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
	RoadMesh->SetupAttachment(SceneRoot);
	RoadMesh->SetCollisionObjectType(ECC_WorldStatic);
	RoadMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RoadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	KerbMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("KerbMesh"));
	KerbMesh->SetupAttachment(SceneRoot);
	KerbMesh->SetCollisionObjectType(ECC_WorldStatic);
	KerbMesh->SetCollisionResponseToAllChannels(ECR_Block);
	KerbMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GroundCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("GroundCollision"));
	GroundCollision->SetupAttachment(SceneRoot);
	GroundCollision->SetCollisionProfileName(TEXT("BlockAll"));
	GroundCollision->SetCollisionObjectType(ECC_WorldStatic);

	GroundVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundVisual"));
	GroundVisual->SetupAttachment(SceneRoot);
	GroundVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StartFinishVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartFinishVisual"));
	StartFinishVisual->SetupAttachment(SceneRoot);
	StartFinishVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		GroundVisual->SetStaticMesh(PlaneMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		StartFinishVisual->SetStaticMesh(CubeMesh.Object);
	}
}

void AApexTrackActor::BeginPlay()
{
	Super::BeginPlay();
	BuildTrack();
}

void AApexTrackActor::BuildTrack()
{
	if (bTrackBuilt)
	{
		return;
	}

	if (!LoadLayout())
	{
		UE_LOG(LogTemp, Error, TEXT("APEX Circuit: could not load Content/Data/ApexCircuitLayout.json."));
		return;
	}

	BuildRoundedCenterline();
	if (CenterlinePoints.Num() < 3)
	{
		UE_LOG(LogTemp, Error, TEXT("APEX Circuit: route generation produced fewer than three points."));
		return;
	}

	CenterlineSpline->ClearSplinePoints(false);
	for (const FVector& Point : CenterlinePoints)
	{
		CenterlineSpline->AddSplinePoint(Point, ESplineCoordinateSpace::Local, false);
	}
	CenterlineSpline->SetClosedLoop(true, false);
	for (int32 Index = 0; Index < CenterlinePoints.Num(); ++Index)
	{
		CenterlineSpline->SetSplinePointType(Index, ESplinePointType::Linear, false);
	}
	CenterlineSpline->UpdateSpline();

	BuildSurfaceMeshes();
	BuildTimingData();
	bTrackBuilt = true;

	UE_LOG(LogTemp, Display, TEXT("APEX Circuit loaded: %.1f m, %d samples, %d checkpoints, %d DRS zones."), GetTrackLengthMetres(), CenterlinePoints.Num(), CheckpointDistancesCm.Num(), DrsZones.Num());
}

bool AApexTrackActor::LoadLayout()
{
	FString JsonText;
	const FString LayoutPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Data/ApexCircuitLayout.json"));
	if (!FFileHelper::LoadFileToString(JsonText, *LayoutPath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* OriginValues = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* PointValues = nullptr;
	if (!Root->TryGetArrayField(TEXT("source_origin"), OriginValues) || OriginValues == nullptr || OriginValues->Num() != 2 || !Root->TryGetArrayField(TEXT("source_points"), PointValues) || !ReadPointArray(PointValues, SourcePoints))
	{
		return false;
	}

	SourceOrigin = FVector2D(static_cast<float>((*OriginValues)[0]->AsNumber()), static_cast<float>((*OriginValues)[1]->AsNumber()));
	MetresPerSourceUnit = static_cast<float>(Root->GetNumberField(TEXT("metres_per_source_unit")));
	SampleStepCm = static_cast<float>(Root->GetNumberField(TEXT("sample_step_metres"))) * 100.0f;
	RoadHalfWidthCm = static_cast<float>(Root->GetNumberField(TEXT("road_half_width_metres"))) * 100.0f;
	KerbWidthCm = static_cast<float>(Root->GetNumberField(TEXT("kerb_width_metres"))) * 100.0f;
	KerbSurfaceWidthCm = static_cast<float>(Root->GetNumberField(TEXT("kerb_surface_width_metres"))) * 100.0f;
	KerbHeightCm = static_cast<float>(Root->GetNumberField(TEXT("kerb_height_metres"))) * 100.0f;

	CheckpointFractions.Reset();
	const TArray<TSharedPtr<FJsonValue>>* CheckpointValues = nullptr;
	if (Root->TryGetArrayField(TEXT("checkpoint_fractions"), CheckpointValues) && CheckpointValues != nullptr)
	{
		for (const TSharedPtr<FJsonValue>& Value : *CheckpointValues)
		{
			CheckpointFractions.Add(static_cast<float>(Value->AsNumber()));
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* SectorValues = nullptr;
	if (!Root->TryGetArrayField(TEXT("sector_anchors"), SectorValues) || !ReadPointArray(SectorValues, SectorAnchors))
	{
		return false;
	}

	DrsFractions.Reset();
	const TArray<TSharedPtr<FJsonValue>>* DrsValues = nullptr;
	if (!Root->TryGetArrayField(TEXT("drs_zones"), DrsValues) || DrsValues == nullptr)
	{
		return false;
	}
	for (const TSharedPtr<FJsonValue>& Value : *DrsValues)
	{
		FVector2D Zone;
		if (!ReadPoint(Value, Zone))
		{
			return false;
		}
		DrsFractions.Add(TPair<float, float>(Zone.X, Zone.Y));
	}

	GravelPolygons.Reset();
	const TArray<TSharedPtr<FJsonValue>>* GravelValues = nullptr;
	if (!Root->TryGetArrayField(TEXT("source_gravel"), GravelValues) || GravelValues == nullptr)
	{
		return false;
	}
	for (const TSharedPtr<FJsonValue>& Value : *GravelValues)
	{
		const TArray<TSharedPtr<FJsonValue>>* PolygonValues = nullptr;
		if (!Value.IsValid() || !Value->TryGetArray(PolygonValues))
		{
			return false;
		}

		TArray<FVector2D> Polygon;
		if (!ReadPointArray(PolygonValues, Polygon))
		{
			return false;
		}

		for (FVector2D& Point : Polygon)
		{
			const FVector Local = SourceToLocal(Point);
			Point = FVector2D(Local.X, Local.Y);
		}
		GravelPolygons.Add(MoveTemp(Polygon));
	}

	return true;
}

void AApexTrackActor::BuildRoundedCenterline()
{
	TArray<FVector> RoundedPoints;
	for (int32 Index = 0; Index < SourcePoints.Num(); ++Index)
	{
		const FVector2D& Point = SourcePoints[Index];
		const FVector2D Incoming = SourcePoints[(Index - 1 + SourcePoints.Num()) % SourcePoints.Num()] - Point;
		const FVector2D Outgoing = SourcePoints[(Index + 1) % SourcePoints.Num()] - Point;
		const float Radius = FMath::Min(90.0f, FMath::Min(Incoming.Length() * 0.22f, Outgoing.Length() * 0.22f));
		const FVector2D Entry = Point + Incoming.GetSafeNormal() * Radius;
		const FVector2D Exit = Point + Outgoing.GetSafeNormal() * Radius;

		RoundedPoints.Add(SourceToLocal(Entry));
		for (int32 Step = 1; Step < 25; ++Step)
		{
			const float Amount = static_cast<float>(Step) / 24.0f;
			const float Inverse = 1.0f - Amount;
			const FVector2D BezierPoint = Entry * (Inverse * Inverse) + Point * (2.0f * Inverse * Amount) + Exit * (Amount * Amount);
			RoundedPoints.Add(SourceToLocal(BezierPoint));
		}
	}

	int32 StartIndex = 0;
	for (int32 Index = 0; Index < RoundedPoints.Num(); ++Index)
	{
		const FVector& First = RoundedPoints[Index];
		const FVector& Second = RoundedPoints[(Index + 1) % RoundedPoints.Num()];
		if (FMath::IsNearlyZero(First.Y, KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(Second.Y, KINDA_SMALL_NUMBER) && First.X <= 0.0f && Second.X > 0.0f)
		{
			RoundedPoints.Insert(FVector::ZeroVector, Index + 1);
			StartIndex = Index + 1;
			break;
		}
	}

	CenterlinePoints.Reset();
	for (int32 Cursor = 0; Cursor < RoundedPoints.Num(); ++Cursor)
	{
		const int32 Index = (StartIndex + Cursor) % RoundedPoints.Num();
		const FVector& First = RoundedPoints[Index];
		const FVector& Second = RoundedPoints[(Index + 1) % RoundedPoints.Num()];
		const int32 Count = FMath::Max(1, FMath::CeilToInt(FVector::Distance(First, Second) / SampleStepCm));
		for (int32 Step = 1; Step <= Count; ++Step)
		{
			const FVector Point = FMath::Lerp(First, Second, static_cast<float>(Step) / static_cast<float>(Count));
			if (CenterlinePoints.IsEmpty() || FVector::DistSquared(CenterlinePoints.Last(), Point) > 0.0001f)
			{
				CenterlinePoints.Add(Point);
			}
		}
	}

	TrackLengthCm = 0.0f;
	CumulativeDistancesCm.Reset();
	Tangents.Reset();
	LateralNormals.Reset();
	KerbSegments.Reset();
	for (int32 Index = 0; Index < CenterlinePoints.Num(); ++Index)
	{
		CumulativeDistancesCm.Add(TrackLengthCm);
		const FVector& Current = CenterlinePoints[Index];
		const FVector& Next = CenterlinePoints[(Index + 1) % CenterlinePoints.Num()];
		TrackLengthCm += FVector::Distance(Current, Next);
		const FVector& Previous = CenterlinePoints[(Index - 1 + CenterlinePoints.Num()) % CenterlinePoints.Num()];
		const FVector Tangent = ((Current - Previous).GetSafeNormal() + (Next - Current).GetSafeNormal()).GetSafeNormal();
		Tangents.Add(Tangent);
		LateralNormals.Add(FVector(Tangent.Y, -Tangent.X, 0.0f).GetSafeNormal());
	}

	for (int32 Index = 0; Index < CenterlinePoints.Num(); ++Index)
	{
		KerbSegments.Add(CurvatureAtIndex(Index) >= 0.013f && (CurvatureAtIndex((Index - 1 + CenterlinePoints.Num()) % CenterlinePoints.Num()) >= 0.00975f || CurvatureAtIndex((Index + 1) % CenterlinePoints.Num()) >= 0.00975f));
	}
}

void AApexTrackActor::BuildSurfaceMeshes()
{
	TArray<FVector> RoadVertices;
	TArray<int32> RoadTriangles;
	TArray<FVector> RoadNormals;
	TArray<FVector2D> RoadUvs;
	TArray<FLinearColor> RoadColors;
	TArray<FProcMeshTangent> RoadTangents;
	const int32 PointCount = CenterlinePoints.Num();

	for (int32 Index = 0; Index <= PointCount; ++Index)
	{
		const int32 Wrapped = Index % PointCount;
		const FVector& Center = CenterlinePoints[Wrapped];
		const FVector& Lateral = LateralNormals[Wrapped];
		RoadVertices.Add(Center - Lateral * RoadHalfWidthCm);
		RoadVertices.Add(Center + Lateral * RoadHalfWidthCm);
		RoadNormals.Add(FVector::UpVector);
		RoadNormals.Add(FVector::UpVector);
		const float Progress = Index == PointCount ? TrackLengthCm : CumulativeDistancesCm[Wrapped];
		RoadUvs.Add(FVector2D(-RoadHalfWidthCm / 100.0f, Progress / 100.0f));
		RoadUvs.Add(FVector2D(RoadHalfWidthCm / 100.0f, Progress / 100.0f));
		RoadColors.Add(FLinearColor::Gray);
		RoadColors.Add(FLinearColor::Gray);
		RoadTangents.Add(FProcMeshTangent(Tangents[Wrapped], false));
		RoadTangents.Add(FProcMeshTangent(Tangents[Wrapped], false));

		if (Index < PointCount)
		{
			const int32 Base = Index * 2;
			RoadTriangles.Append({Base, Base + 2, Base + 1, Base + 1, Base + 2, Base + 3});
		}
	}

	RoadMesh->ClearAllMeshSections();
	RoadMesh->CreateMeshSection_LinearColor(0, RoadVertices, RoadTriangles, RoadNormals, RoadUvs, RoadColors, RoadTangents, true);

	TArray<FVector> KerbVertices;
	TArray<int32> KerbTriangles;
	TArray<FVector> KerbNormals;
	TArray<FVector2D> KerbUvs;
	TArray<FLinearColor> KerbColors;
	TArray<FProcMeshTangent> KerbTangents;
	for (int32 Index = 0; Index < PointCount; ++Index)
	{
		if (!IsKerbSegment(Index))
		{
			continue;
		}

		const int32 Next = (Index + 1) % PointCount;
		for (const float Side : {-1.0f, 1.0f})
		{
			const int32 Base = KerbVertices.Num();
			const FVector A = CenterlinePoints[Index] + LateralNormals[Index] * (Side * RoadHalfWidthCm) + FVector::UpVector * KerbHeightCm;
			const FVector B = CenterlinePoints[Index] + LateralNormals[Index] * (Side * (RoadHalfWidthCm + KerbSurfaceWidthCm)) + FVector::UpVector * KerbHeightCm;
			const FVector C = CenterlinePoints[Next] + LateralNormals[Next] * (Side * RoadHalfWidthCm) + FVector::UpVector * KerbHeightCm;
			const FVector D = CenterlinePoints[Next] + LateralNormals[Next] * (Side * (RoadHalfWidthCm + KerbSurfaceWidthCm)) + FVector::UpVector * KerbHeightCm;
			KerbVertices.Append({A, B, C, D});
			KerbTriangles.Append({Base, Base + 2, Base + 1, Base + 1, Base + 2, Base + 3});
			for (int32 Vertex = 0; Vertex < 4; ++Vertex)
			{
				KerbNormals.Add(FVector::UpVector);
				KerbUvs.Add(FVector2D(Vertex % 2, CumulativeDistancesCm[Index] / 100.0f));
				KerbColors.Add((Index / 8) % 2 == 0 ? FLinearColor::White : FLinearColor(0.8f, 0.04f, 0.04f));
				KerbTangents.Add(FProcMeshTangent(Tangents[Index], false));
			}
		}
	}
	KerbMesh->ClearAllMeshSections();
	if (!KerbVertices.IsEmpty())
	{
		KerbMesh->CreateMeshSection_LinearColor(0, KerbVertices, KerbTriangles, KerbNormals, KerbUvs, KerbColors, KerbTangents, true);
	}

	FVector MinBounds(FLT_MAX);
	FVector MaxBounds(-FLT_MAX);
	for (const FVector& Point : CenterlinePoints)
	{
		MinBounds.X = FMath::Min(MinBounds.X, Point.X);
		MinBounds.Y = FMath::Min(MinBounds.Y, Point.Y);
		MaxBounds.X = FMath::Max(MaxBounds.X, Point.X);
		MaxBounds.Y = FMath::Max(MaxBounds.Y, Point.Y);
	}
	const FVector GroundCenter = (MinBounds + MaxBounds) * 0.5f + FVector(0.0f, 0.0f, -70.0f);
	const FVector GroundExtent((MaxBounds.X - MinBounds.X) * 0.5f + 5000.0f, (MaxBounds.Y - MinBounds.Y) * 0.5f + 5000.0f, 50.0f);
	GroundCollision->SetRelativeLocation(GroundCenter);
	GroundCollision->SetBoxExtent(GroundExtent);
	GroundVisual->SetRelativeLocation(GroundCenter + FVector(0.0f, 0.0f, 45.0f));
	GroundVisual->SetRelativeScale3D(FVector(GroundExtent.X / 50.0f, GroundExtent.Y / 50.0f, 1.0f));

	const FVector StartLocation = CenterlineSpline->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::Local);
	const FVector StartDirection = CenterlineSpline->GetDirectionAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::Local);
	const FVector StartLateral(StartDirection.Y, -StartDirection.X, 0.0f);
	StartFinishVisual->SetRelativeLocation(StartLocation + FVector::UpVector * 2.0f);
	StartFinishVisual->SetRelativeRotation(FRotationMatrix::MakeFromXY(StartDirection, StartLateral).Rotator());
	StartFinishVisual->SetRelativeScale3D(FVector(0.12f, RoadHalfWidthCm * 2.0f / 100.0f, 0.025f));
}

void AApexTrackActor::BuildTimingData()
{
	CheckpointDistancesCm.Reset();
	for (const float Fraction : CheckpointFractions)
	{
		CheckpointDistancesCm.Add(TrackLengthCm * Fraction);
	}

	SectorDistancesCm.Reset();
	for (const FVector2D& Anchor : SectorAnchors)
	{
		SectorDistancesCm.Add(GetProgressAtWorldLocation(GetActorTransform().TransformPosition(SourceToLocal(Anchor))));
	}
	SectorDistancesCm.Sort();
	SectorDistancesCm.Add(TrackLengthCm);

	DrsZones.Reset();
	for (const TPair<float, float>& Fractions : DrsFractions)
	{
		FApexDrsZone& Zone = DrsZones.AddDefaulted_GetRef();
		Zone.StartDistanceCm = TrackLengthCm * Fractions.Key;
		Zone.EndDistanceCm = TrackLengthCm * Fractions.Value;
		Zone.DetectionDistanceCm = WrapDistance(Zone.StartDistanceCm - 6500.0f, TrackLengthCm);
	}
}

FVector AApexTrackActor::SourceToLocal(const FVector2D& SourcePoint) const
{
	const FVector2D Metres = (SourcePoint - SourceOrigin) * MetresPerSourceUnit;
	return FVector(Metres.X * 100.0f, Metres.Y * 100.0f, 0.0f);
}

float AApexTrackActor::GetProgressAtWorldLocation(const FVector& WorldLocation) const
{
	if (!bTrackBuilt && CenterlinePoints.IsEmpty())
	{
		return 0.0f;
	}
	const float InputKey = CenterlineSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
	return WrapDistance(CenterlineSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey), TrackLengthCm);
}

FApexSurfaceSample AApexTrackActor::SampleSurfaceAtWorldLocation(const FVector& WorldLocation) const
{
	FApexSurfaceSample Sample;
	if (CenterlinePoints.IsEmpty())
	{
		return Sample;
	}

	const float InputKey = CenterlineSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
	Sample.ProgressCm = WrapDistance(CenterlineSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey), TrackLengthCm);
	const FVector Center = CenterlineSpline->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
	const FVector Direction = CenterlineSpline->GetDirectionAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).GetSafeNormal();
	const FVector Lateral(Direction.Y, -Direction.X, 0.0f);
	Sample.LateralOffsetCm = FVector::DotProduct(WorldLocation - Center, Lateral);
	const float AbsoluteOffset = FMath::Abs(Sample.LateralOffsetCm);
	const int32 PointIndex = FMath::FloorToInt(InputKey + 0.5f) % CenterlinePoints.Num();

	if (AbsoluteOffset <= RoadHalfWidthCm)
	{
		Sample.Surface = EApexSurface::Asphalt;
		Sample.Grip = 1.0f;
		Sample.RollingResistance = 0.012f;
	}
	else if (IsKerbSegment(PointIndex) && AbsoluteOffset <= RoadHalfWidthCm + KerbSurfaceWidthCm)
	{
		Sample.Surface = EApexSurface::Kerb;
		Sample.Grip = 0.58f;
		Sample.RollingResistance = 0.032f;
	}
	else if (IsInsideGravel(FVector2D(WorldLocation.X, WorldLocation.Y)))
	{
		Sample.Surface = EApexSurface::Gravel;
		Sample.Grip = 0.52f;
		Sample.RollingResistance = 0.19f;
	}
	else
	{
		Sample.Surface = EApexSurface::Grass;
		Sample.Grip = 0.38f;
		Sample.RollingResistance = 0.072f;
	}

	return Sample;
}

FTransform AApexTrackActor::GetSpawnTransformAtDistance(float DistanceCm, float LateralOffsetCm) const
{
	const float WrappedDistance = WrapDistance(DistanceCm, TrackLengthCm);
	const FVector Location = CenterlineSpline->GetLocationAtDistanceAlongSpline(WrappedDistance, ESplineCoordinateSpace::World);
	const FVector Direction = CenterlineSpline->GetDirectionAtDistanceAlongSpline(WrappedDistance, ESplineCoordinateSpace::World).GetSafeNormal();
	const FVector Lateral(Direction.Y, -Direction.X, 0.0f);
	const FQuat Rotation = FRotationMatrix::MakeFromXZ(Direction, FVector::UpVector).ToQuat();
	return FTransform(Rotation, Location + Lateral * FMath::Clamp(LateralOffsetCm, -RoadHalfWidthCm + 200.0f, RoadHalfWidthCm - 200.0f) + FVector::UpVector * 80.0f);
}

int32 AApexTrackActor::GetSectorForDistance(float DistanceCm) const
{
	const float WrappedDistance = WrapDistance(DistanceCm, TrackLengthCm);
	for (int32 Index = 0; Index < SectorDistancesCm.Num(); ++Index)
	{
		if (WrappedDistance < SectorDistancesCm[Index])
		{
			return Index + 1;
		}
	}
	return 3;
}

bool AApexTrackActor::IsDistanceInDrsZone(float DistanceCm) const
{
	const float WrappedDistance = WrapDistance(DistanceCm, TrackLengthCm);
	for (const FApexDrsZone& Zone : DrsZones)
	{
		if (Zone.StartDistanceCm <= Zone.EndDistanceCm)
		{
			if (WrappedDistance >= Zone.StartDistanceCm && WrappedDistance <= Zone.EndDistanceCm)
			{
				return true;
			}
		}
		else if (WrappedDistance >= Zone.StartDistanceCm || WrappedDistance <= Zone.EndDistanceCm)
		{
			return true;
		}
	}
	return false;
}

float AApexTrackActor::CurvatureAtIndex(int32 Index) const
{
	const int32 PointCount = CenterlinePoints.Num();
	const FVector& Here = CenterlinePoints[Index];
	const FVector& Next = CenterlinePoints[(Index + 1) % PointCount];
	const FVector& Previous = CenterlinePoints[(Index - 1 + PointCount) % PointCount];
	const FVector PreviousDirection = (Here - Previous).GetSafeNormal();
	const FVector NextDirection = (Next - Here).GetSafeNormal();
	const float Angle = FMath::Abs(FMath::Atan2(FVector::CrossProduct(PreviousDirection, NextDirection).Z, FVector::DotProduct(PreviousDirection, NextDirection)));
	return Angle / FMath::Max(1.0f, FVector::Distance(Here, Next) / 100.0f);
}

bool AApexTrackActor::IsKerbSegment(int32 Index) const
{
	return KerbSegments.IsValidIndex(Index) && KerbSegments[Index];
}

bool AApexTrackActor::IsInsideGravel(const FVector2D& WorldPoint) const
{
	const FVector LocalPoint = GetActorTransform().InverseTransformPosition(FVector(WorldPoint.X, WorldPoint.Y, 0.0f));
	for (const TArray<FVector2D>& Polygon : GravelPolygons)
	{
		if (IsPointInPolygon(FVector2D(LocalPoint.X, LocalPoint.Y), Polygon))
		{
			return true;
		}
	}
	return false;
}

bool AApexTrackActor::IsPointInPolygon(const FVector2D& Point, const TArray<FVector2D>& Polygon)
{
	bool bInside = false;
	for (int32 Index = 0, Previous = Polygon.Num() - 1; Index < Polygon.Num(); Previous = Index++)
	{
		const FVector2D& A = Polygon[Index];
		const FVector2D& B = Polygon[Previous];
		const float Denominator = B.Y - A.Y;
		if (FMath::IsNearlyZero(Denominator))
		{
			continue;
		}
		const bool bIntersects = ((A.Y > Point.Y) != (B.Y > Point.Y)) && (Point.X < (B.X - A.X) * (Point.Y - A.Y) / Denominator + A.X);
		if (bIntersects)
		{
			bInside = !bInside;
		}
	}
	return bInside;
}

float AApexTrackActor::WrapDistance(float Distance, float Length)
{
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}
	return FMath::Fmod(FMath::Fmod(Distance, Length) + Length, Length);
}
