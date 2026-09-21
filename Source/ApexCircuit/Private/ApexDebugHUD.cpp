#include "ApexDebugHUD.h"

#include "ApexCircuitGameMode.h"
#include "ApexFormulaCar.h"
#include "ApexRaceDirector.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void AApexDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	const AApexCircuitGameMode* GameMode = GetWorld() != nullptr ? GetWorld()->GetAuthGameMode<AApexCircuitGameMode>() : nullptr;
	const AApexFormulaCar* Car = GameMode != nullptr ? GameMode->GetPlayerCar() : nullptr;
	const AApexRaceDirector* RaceDirector = GameMode != nullptr ? GameMode->GetRaceDirector() : nullptr;
	if (Car == nullptr || RaceDirector == nullptr || Canvas == nullptr)
	{
		return;
	}

	UFont* Font = GEngine->GetLargeFont();
	UFont* SmallFont = GEngine->GetSmallFont();
	const float SpeedBoxWidth = 280.0f;
	const float SpeedBoxHeight = 155.0f;
	const float SpeedBoxX = Canvas->ClipX - SpeedBoxWidth - 36.0f;
	const float SpeedBoxY = Canvas->ClipY - SpeedBoxHeight - 38.0f;
	DrawRect(FLinearColor(0.01f, 0.02f, 0.025f, 0.78f), SpeedBoxX, SpeedBoxY, SpeedBoxWidth, SpeedBoxHeight);
	DrawText(FString::Printf(TEXT("%03d"), FMath::RoundToInt(Car->GetSpeedKph())), FLinearColor::White, SpeedBoxX + 24.0f, SpeedBoxY + 12.0f, Font, 1.65f, false);
	DrawText(TEXT("KM/H"), FLinearColor(0.58f, 0.82f, 0.88f), SpeedBoxX + 188.0f, SpeedBoxY + 69.0f, SmallFont, 1.0f, false);
	DrawText(FString::Printf(TEXT("GEAR %d    %5.0f RPM"), Car->GetGear(), Car->GetEngineRpm()), FLinearColor::White, SpeedBoxX + 24.0f, SpeedBoxY + 108.0f, SmallFont, 1.15f, false);

	DrawRect(FLinearColor(0.01f, 0.02f, 0.025f, 0.78f), 32.0f, 32.0f, 355.0f, 150.0f);
	DrawText(FString::Printf(TEXT("LAP %d    SECTOR %d"), RaceDirector->GetCurrentLap(), RaceDirector->GetCurrentSector()), FLinearColor::White, 52.0f, 50.0f, SmallFont, 1.2f, false);
	DrawText(FString::Printf(TEXT("LAP TIME  %s"), *FormatTime(RaceDirector->GetCurrentLapTimeSeconds())), FLinearColor::White, 52.0f, 82.0f, SmallFont, 1.1f, false);
	DrawText(FString::Printf(TEXT("LAST      %s"), *FormatTime(RaceDirector->GetLastLapTimeSeconds())), FLinearColor(0.64f, 0.84f, 0.72f), 52.0f, 110.0f, SmallFont, 1.1f, false);
	DrawText(FString::Printf(TEXT("SURFACE   %s"), *SurfaceToString(static_cast<uint8>(Car->GetCurrentSurface()))), FLinearColor(0.82f, 0.82f, 0.82f), 52.0f, 138.0f, SmallFont, 1.0f, false);

	const FString DrsText = Car->IsDrsOpen() ? TEXT("DRS OPEN") : (Car->IsDrsAvailable() ? TEXT("DRS READY") : TEXT("DRS"));
	const FLinearColor DrsColor = Car->IsDrsOpen() ? FLinearColor(0.25f, 1.0f, 0.55f) : (Car->IsDrsAvailable() ? FLinearColor(0.95f, 0.82f, 0.15f) : FLinearColor(0.35f, 0.4f, 0.43f));
	DrawText(DrsText, DrsColor, Canvas->ClipX * 0.5f - 46.0f, 38.0f, SmallFont, 1.25f, false);
	DrawText(FString::Printf(TEXT("ERS %.1f MJ"), Car->GetEnergyMj()), FLinearColor(0.46f, 0.76f, 1.0f), Canvas->ClipX * 0.5f - 48.0f, 67.0f, SmallFont, 1.0f, false);
}

FString AApexDebugHUD::SurfaceToString(uint8 SurfaceValue)
{
	switch (static_cast<EApexSurface>(SurfaceValue))
	{
	case EApexSurface::Kerb:
		return TEXT("KERB");
	case EApexSurface::Grass:
		return TEXT("GRASS");
	case EApexSurface::Gravel:
		return TEXT("GRAVEL");
	default:
		return TEXT("ASPHALT");
	}
}

FString AApexDebugHUD::FormatTime(float Seconds)
{
	if (Seconds <= 0.0f)
	{
		return TEXT("--:--.---");
	}
	const int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	const float RemainingSeconds = Seconds - Minutes * 60.0f;
	return FString::Printf(TEXT("%d:%06.3f"), Minutes, RemainingSeconds);
}
