#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ApexDebugHUD.generated.h"

UCLASS()
class APEXCIRCUIT_API AApexDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	static FString SurfaceToString(uint8 SurfaceValue);
	static FString FormatTime(float Seconds);
};
