#include "ApexPresentationDirector.h"

#include "ApexFormulaCar.h"
#include "ApexTrackActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	void ConfigureInstances(UHierarchicalInstancedStaticMeshComponent* Component, UStaticMesh* Mesh, UMaterialInterface* Material, bool bCastShadows = true)
	{
		Component->SetStaticMesh(Mesh);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCastShadow(bCastShadows);
		Component->bAffectDistanceFieldLighting = true;
		if (Material != nullptr)
		{
			Component->SetMaterial(0, Material);
		}
	}
}

AApexPresentationDirector::AApexPresentationDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	SceneRoot->SetMobility(EComponentMobility::Static);

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(SceneRoot);
	Sun->SetMobility(EComponentMobility::Movable);
	Sun->SetAtmosphereSunLight(true);
	Sun->SetIntensity(9.0f);
	Sun->SetLightColor(FLinearColor(1.0f, 0.88f, 0.72f));
	Sun->SetLightSourceAngle(0.55f);
	Sun->SetCastShadows(true);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(SceneRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetIntensity(0.72f);
	SkyLight->bRealTimeCapture = true;
	SkyLight->bLowerHemisphereIsBlack = false;

	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
	Atmosphere->SetupAttachment(SceneRoot);

	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("VolumetricFog"));
	Fog->SetupAttachment(SceneRoot);
	Fog->SetFogDensity(0.012f);
	Fog->SetFogHeightFalloff(0.18f);
	Fog->SetVolumetricFog(true);
	Fog->SetVolumetricFogExtinctionScale(0.45f);
	Fog->SetVolumetricFogDistance(90000.0f);

	Clouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
	Clouds->SetupAttachment(SceneRoot);
	Clouds->SetLayerBottomAltitude(2.7f);
	Clouds->SetLayerHeight(8.0f);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CloudMaterial(TEXT("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst.m_SimpleVolumetricCloud_Inst"));
	if (CloudMaterial.Succeeded())
	{
		Clouds->SetMaterial(CloudMaterial.Object);
	}

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CinematicPostProcess"));
	PostProcess->SetupAttachment(SceneRoot);
	PostProcess->bUnbound = true;
	PostProcess->BlendWeight = 1.0f;
	PostProcess->Settings.bOverride_BloomIntensity = true;
	PostProcess->Settings.BloomIntensity = 0.32f;
	PostProcess->Settings.bOverride_VignetteIntensity = true;
	PostProcess->Settings.VignetteIntensity = 0.18f;
	PostProcess->Settings.bOverride_MotionBlurAmount = true;
	PostProcess->Settings.MotionBlurAmount = 0.34f;
	PostProcess->Settings.bOverride_MotionBlurMax = true;
	PostProcess->Settings.MotionBlurMax = 7.0f;
	PostProcess->Settings.bOverride_FilmSlope = true;
	PostProcess->Settings.FilmSlope = 0.91f;
	PostProcess->Settings.bOverride_FilmToe = true;
	PostProcess->Settings.FilmToe = 0.48f;
	PostProcess->Settings.bOverride_FilmShoulder = true;
	PostProcess->Settings.FilmShoulder = 0.25f;
	PostProcess->Settings.bOverride_SceneFringeIntensity = true;
	PostProcess->Settings.SceneFringeIntensity = 0.18f;
	PostProcess->Settings.bOverride_AutoExposureMethod = true;
	PostProcess->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
	PostProcess->Settings.bOverride_AutoExposureMinBrightness = true;
	PostProcess->Settings.AutoExposureMinBrightness = -3.0f;
	PostProcess->Settings.bOverride_AutoExposureMaxBrightness = true;
	PostProcess->Settings.AutoExposureMaxBrightness = 12.0f;
	PostProcess->Settings.bOverride_AutoExposureSpeedUp = true;
	PostProcess->Settings.AutoExposureSpeedUp = 3.0f;
	PostProcess->Settings.bOverride_AutoExposureSpeedDown = true;
	PostProcess->Settings.AutoExposureSpeedDown = 1.25f;
	PostProcess->Settings.bOverride_WhiteTemp = true;
	PostProcess->Settings.WhiteTemp = 5900.0f;
	PostProcess->Settings.bOverride_DepthOfFieldEnabled = true;
	PostProcess->Settings.DepthOfFieldEnabled = true;
	PostProcess->Settings.bOverride_DepthOfFieldFstop = true;
	PostProcess->Settings.DepthOfFieldFstop = 5.6f;
	PostProcess->Settings.bOverride_DepthOfFieldFocalDistance = true;
	PostProcess->Settings.DepthOfFieldFocalDistance = 2200.0f;
	PostProcess->Settings.bOverride_LensFlareIntensity = true;
	PostProcess->Settings.LensFlareIntensity = 0.18f;
	PostProcess->Settings.bOverride_LensFlareThreshold = true;
	PostProcess->Settings.LensFlareThreshold = 7.0f;
	static ConstructorHelpers::FObjectFinder<UTexture> LensDirt(TEXT("/Game/Art/Textures/wet_asphalt_microdetail.wet_asphalt_microdetail"));
	if (LensDirt.Succeeded())
	{
		PostProcess->Settings.bOverride_BloomDirtMask = true;
		PostProcess->Settings.BloomDirtMask = LensDirt.Object;
		PostProcess->Settings.bOverride_BloomDirtMaskIntensity = true;
		PostProcess->Settings.BloomDirtMaskIntensity = 0.06f;
	}

	Armco = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Armco"));
	Armco->SetupAttachment(SceneRoot);
	FencePosts = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("FencePosts"));
	FencePosts->SetupAttachment(SceneRoot);
	LightPoles = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("LightPoles"));
	LightPoles->SetupAttachment(SceneRoot);
	LightHeads = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("LightHeads"));
	LightHeads->SetupAttachment(SceneRoot);
	Grandstands = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Grandstands"));
	Grandstands->SetupAttachment(SceneRoot);
	Buildings = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Buildings"));
	Buildings->SetupAttachment(SceneRoot);
	SponsorBoards = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("SponsorBoards"));
	SponsorBoards->SetupAttachment(SceneRoot);
	PitWall = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("PitWall"));
	PitWall->SetupAttachment(SceneRoot);
	MarshalPosts = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("MarshalPosts"));
	MarshalPosts->SetupAttachment(SceneRoot);
	TrackCameras = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("TrackCameras"));
	TrackCameras->SetupAttachment(SceneRoot);
	Vegetation = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Vegetation"));
	Vegetation->SetupAttachment(SceneRoot);
	BroadleafTrees = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("BroadleafTrees"));
	BroadleafTrees->SetupAttachment(SceneRoot);
	PineTrees = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("PineTrees"));
	PineTrees->SetupAttachment(SceneRoot);
	RainStreaks = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("RainStreaks"));
	RainStreaks->SetupAttachment(SceneRoot);
	RainStreaks->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BroadleafMesh(TEXT("/Game/Environment/Nature/IslandTree/island_tree/StaticMeshes/SM_IslandTree.SM_IslandTree"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PineMesh(TEXT("/Game/Environment/Nature/PineSaplings/pine_saplings/StaticMeshes/pine_sapling_small_a.pine_sapling_small_a"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BarrierMaterial(TEXT("/Game/Art/Materials/M_ApexBarrier.M_ApexBarrier"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> StructureMaterial(TEXT("/Game/Art/Materials/M_ApexStructure.M_ApexStructure"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LightMaterial(TEXT("/Game/Art/Materials/M_ApexLight.M_ApexLight"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GrassMaterial(TEXT("/Game/Art/Materials/M_ApexGrass.M_ApexGrass"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SponsorMaterial(TEXT("/Game/Art/Materials/M_ApexSponsor.M_ApexSponsor"));
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> WeatherParameters(TEXT("/Game/Art/Materials/MPC_ApexWeather.MPC_ApexWeather"));
	if (WeatherParameters.Succeeded())
	{
		WeatherCollection = WeatherParameters.Object;
	}
	if (Cube.Succeeded() && Cylinder.Succeeded() && Cone.Succeeded())
	{
		ConfigureInstances(Armco, Cube.Object, BarrierMaterial.Object);
		ConfigureInstances(FencePosts, Cylinder.Object, BarrierMaterial.Object);
		ConfigureInstances(LightPoles, Cylinder.Object, StructureMaterial.Object);
		ConfigureInstances(LightHeads, Cube.Object, LightMaterial.Object);
		ConfigureInstances(Grandstands, Cube.Object, StructureMaterial.Object);
		ConfigureInstances(Buildings, Cube.Object, StructureMaterial.Object);
		ConfigureInstances(SponsorBoards, Cube.Object, SponsorMaterial.Object);
		ConfigureInstances(PitWall, Cube.Object, BarrierMaterial.Object);
		ConfigureInstances(MarshalPosts, Cube.Object, StructureMaterial.Object);
		ConfigureInstances(TrackCameras, Cylinder.Object, StructureMaterial.Object);
		ConfigureInstances(Vegetation, Cone.Object, GrassMaterial.Object);
		ConfigureInstances(BroadleafTrees, BroadleafMesh.Succeeded() ? BroadleafMesh.Object : Cone.Object, nullptr);
		ConfigureInstances(PineTrees, PineMesh.Succeeded() ? PineMesh.Object : Cone.Object, nullptr);
		ConfigureInstances(RainStreaks, Cylinder.Object, LightMaterial.Object, false);
		RainStreaks->SetMobility(EComponentMobility::Movable);
	}
}

void AApexPresentationDirector::BeginPlay()
{
	Super::BeginPlay();
	ApplyLighting();
}

void AApexPresentationDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateRain(DeltaSeconds);
}

void AApexPresentationDirector::Initialize(AApexTrackActor* InTrack, AApexFormulaCar* InPlayerCar)
{
	Track = InTrack;
	PlayerCar = InPlayerCar;
	if (!bEnvironmentBuilt && Track.IsValid())
	{
		BuildTracksideEnvironment();
		BuildFloodlights();
		BuildRainField();
		bEnvironmentBuilt = true;
	}
	SetRainIntensity(RainIntensity);
	ApplyLighting();
	UE_LOG(LogTemp, Display, TEXT("APEX Phase 3 presentation initialized: Lumen/VSM world, %d barriers, %d structures, %d rain streaks."), Armco->GetInstanceCount(), Grandstands->GetInstanceCount() + Buildings->GetInstanceCount(), RainStreaks->GetInstanceCount());
}

void AApexPresentationDirector::AddTracksideInstance(UHierarchicalInstancedStaticMeshComponent* Component, float DistanceCm, float LateralCm, const FVector& Scale, float HeightCm, float YawOffset)
{
	if (!Track.IsValid() || Component == nullptr)
	{
		return;
	}
	FTransform Transform = Track->GetSpawnTransformAtDistance(DistanceCm, LateralCm);
	Transform.AddToTranslation(FVector::UpVector * HeightCm);
	Transform.ConcatenateRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(YawOffset)));
	Transform.SetScale3D(Scale);
	Component->AddInstance(Transform, true);
}

void AApexPresentationDirector::BuildTracksideEnvironment()
{
	const float Length = Track->GetTrackLengthCm();
	for (float Distance = 0.0f; Distance < Length; Distance += 1100.0f)
	{
		AddTracksideInstance(Armco, Distance, -1420.0f, FVector(10.6f, 0.18f, 0.46f), 44.0f);
		AddTracksideInstance(Armco, Distance, 1420.0f, FVector(10.6f, 0.18f, 0.46f), 44.0f);
	}
	for (float Distance = 0.0f; Distance < Length; Distance += 2100.0f)
	{
		AddTracksideInstance(FencePosts, Distance, -1510.0f, FVector(0.065f, 0.065f, 1.8f), 180.0f);
		AddTracksideInstance(FencePosts, Distance, 1510.0f, FVector(0.065f, 0.065f, 1.8f), 180.0f);
	}

	const float GrandstandFractions[] = {0.015f, 0.045f, 0.18f, 0.37f, 0.56f, 0.78f, 0.93f};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrandstandFractions); ++Index)
	{
		const float Side = Index % 2 == 0 ? 1.0f : -1.0f;
		const float BaseDistance = Length * GrandstandFractions[Index];
		for (int32 Tier = 0; Tier < 4; ++Tier)
		{
			AddTracksideInstance(Grandstands, BaseDistance, Side * (3300.0f + Tier * 360.0f), FVector(17.0f, 2.8f, 0.45f), 130.0f + Tier * 88.0f);
		}
	}

	for (int32 Index = 0; Index < 8; ++Index)
	{
		AddTracksideInstance(Buildings, 800.0f + Index * 950.0f, -3900.0f, FVector(8.2f, 5.8f, 2.1f), 210.0f);
	}
	AddTracksideInstance(Buildings, Length * 0.012f, 2850.0f, FVector(13.0f, 3.0f, 0.22f), 650.0f);
	for (float Distance = 5500.0f; Distance < Length; Distance += 28500.0f)
	{
		const float Side = FMath::Fmod(Distance / 28500.0f, 2.0f) < 1.0f ? -1.0f : 1.0f;
		AddTracksideInstance(SponsorBoards, Distance, Side * 1700.0f, FVector(4.8f, 0.10f, 1.15f), 150.0f);
	}
	for (float Distance = 0.0f; Distance < 9200.0f; Distance += 900.0f)
	{
		AddTracksideInstance(PitWall, Distance, -1760.0f, FVector(4.3f, 0.16f, 0.62f), 62.0f);
	}
	for (float Distance = 12500.0f; Distance < Length; Distance += 36500.0f)
	{
		const float Side = FMath::Fmod(Distance / 36500.0f, 2.0f) < 1.0f ? -1.0f : 1.0f;
		AddTracksideInstance(MarshalPosts, Distance, Side * 2050.0f, FVector(2.2f, 1.5f, 1.05f), 105.0f);
		AddTracksideInstance(TrackCameras, Distance + 1700.0f, -Side * 1900.0f, FVector(0.07f, 0.07f, 2.6f), 260.0f);
	}

	FRandomStream Random(202603);
	for (int32 Index = 0; Index < 180; ++Index)
	{
		const float Distance = Random.FRandRange(0.0f, Length);
		const float Side = Random.FRand() > 0.5f ? 1.0f : -1.0f;
		const float Offset = Side * Random.FRandRange(5200.0f, 11500.0f);
		const float Height = Random.FRandRange(0.72f, 1.35f);
		UHierarchicalInstancedStaticMeshComponent* NatureComponent = Index % 3 == 0 ? PineTrees.Get() : BroadleafTrees.Get();
		AddTracksideInstance(NatureComponent, Distance, Offset, FVector(Height), 0.0f, Random.FRandRange(0.0f, 360.0f));
	}
}

void AApexPresentationDirector::BuildFloodlights()
{
	const float Length = Track->GetTrackLengthCm();
	for (float Distance = 0.0f; Distance < Length; Distance += 12500.0f)
	{
		for (const float Side : {-1.0f, 1.0f})
		{
			AddTracksideInstance(LightPoles, Distance, Side * 2450.0f, FVector(0.11f, 0.11f, 6.5f), 650.0f);
			AddTracksideInstance(LightHeads, Distance, Side * 2250.0f, FVector(1.8f, 0.32f, 0.16f), 1280.0f);
			USpotLightComponent* Light = NewObject<USpotLightComponent>(this);
			Light->SetupAttachment(SceneRoot);
			Light->RegisterComponent();
			const FTransform TrackTransform = Track->GetSpawnTransformAtDistance(Distance, Side * 2250.0f);
			Light->SetWorldLocation(TrackTransform.GetLocation() + FVector::UpVector * 1250.0f);
			Light->SetWorldRotation((TrackTransform.GetLocation() - Light->GetComponentLocation() + TrackTransform.GetRotation().GetForwardVector() * 1200.0f).Rotation());
			Light->SetIntensity(38000.0f);
			Light->SetAttenuationRadius(9200.0f);
			Light->SetInnerConeAngle(31.0f);
			Light->SetOuterConeAngle(54.0f);
			Light->SetLightColor(FLinearColor(1.0f, 0.84f, 0.64f));
			Light->SetCastShadows(true);
			Floodlights.Add(Light);
		}
	}
}

void AApexPresentationDirector::BuildRainField()
{
	FRandomStream Random(1408);
	RainOffsets.Reset();
	RainStreaks->ClearInstances();
	for (int32 Index = 0; Index < 260; ++Index)
	{
		const FVector Offset(Random.FRandRange(-3200.0f, 4200.0f), Random.FRandRange(-3400.0f, 3400.0f), Random.FRandRange(100.0f, 3600.0f));
		RainOffsets.Add(Offset);
		RainStreaks->AddInstance(FTransform(FRotator::ZeroRotator, Offset, FVector(0.012f, 0.012f, Random.FRandRange(0.9f, 2.7f))), true);
	}
}

void AApexPresentationDirector::UpdateRain(float DeltaSeconds)
{
	if (RainIntensity <= KINDA_SMALL_NUMBER || !PlayerCar.IsValid())
	{
		return;
	}
	const FVector Anchor = PlayerCar->GetActorLocation();
	for (int32 Index = 0; Index < RainOffsets.Num(); ++Index)
	{
		FVector& Offset = RainOffsets[Index];
		Offset.Z -= FMath::Lerp(6500.0f, 15500.0f, RainIntensity) * DeltaSeconds;
		Offset.X -= 1900.0f * DeltaSeconds;
		if (Offset.Z < -100.0f)
		{
			Offset.Z += 3800.0f;
		}
		if (Offset.X < -3400.0f)
		{
			Offset.X += 7600.0f;
		}
		const bool bLast = Index == RainOffsets.Num() - 1;
		RainStreaks->UpdateInstanceTransform(Index, FTransform(FRotator(12.0f, 0.0f, 0.0f), Anchor + Offset, FVector(0.012f, 0.012f, FMath::Lerp(0.8f, 2.8f, RainIntensity))), true, bLast, true);
	}
}

void AApexPresentationDirector::SetRainIntensity(float Intensity)
{
	RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (WeatherCollection != nullptr)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, WeatherCollection, TEXT("Wetness"), FMath::Clamp(RainIntensity * 1.15f, 0.0f, 1.0f));
		UKismetMaterialLibrary::SetScalarParameterValue(this, WeatherCollection, TEXT("RainIntensity"), RainIntensity);
	}
	RainStreaks->SetVisibility(RainIntensity > 0.015f, true);
	if (Track.IsValid())
	{
		Track->SetWetness(FMath::Clamp(RainIntensity * 1.15f, 0.0f, 1.0f));
	}
	Fog->SetFogDensity(FMath::Lerp(0.008f, 0.032f, RainIntensity));
	Fog->SetVolumetricFogExtinctionScale(FMath::Lerp(0.35f, 1.4f, RainIntensity));
}

void AApexPresentationDirector::SetTimeOfDay(float Hour)
{
	TimeOfDayHours = FMath::Fmod(Hour + 24.0f, 24.0f);
	ApplyLighting();
}

void AApexPresentationDirector::ApplyLighting()
{
	const float SolarPitch = FMath::Lerp(-12.0f, 76.0f, FMath::Clamp(1.0f - FMath::Abs(TimeOfDayHours - 12.0f) / 7.0f, 0.0f, 1.0f));
	Sun->SetWorldRotation(FRotator(-SolarPitch, -36.0f, 0.0f));
	const float Daylight = FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp((SolarPitch + 7.0f) / 22.0f, 0.0f, 1.0f));
	Sun->SetIntensity(FMath::Lerp(0.08f, 9.5f, Daylight));
	SkyLight->SetIntensity(FMath::Lerp(0.12f, 0.82f, Daylight));
	const bool bFloodlightsOn = Daylight < 0.72f || RainIntensity > 0.58f;
	for (USpotLightComponent* Light : Floodlights)
	{
		if (Light != nullptr)
		{
			Light->SetVisibility(bFloodlightsOn);
		}
	}
}
