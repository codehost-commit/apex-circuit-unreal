#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ApexRaceHUDWidget.generated.h"

class UButton;
class UTextBlock;

/** Native UMG presentation layer. It remains usable without a hand-authored widget Blueprint and can be restyled in Phase 3. */
UCLASS()
class APEXCIRCUIT_API UApexRaceHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION() void StartPractice();
	UFUNCTION() void StartQualifying();
	UFUNCTION() void StartTimeTrial();
	UFUNCTION() void StartRace();
	UFUNCTION() void Restart();
	UFUNCTION() void Recover();
	UFUNCTION() void TogglePause();
	void StartMode(uint8 Mode);
	void SetText(UTextBlock* Target, const FString& Value) const;

	UPROPERTY(Transient) TObjectPtr<UTextBlock> SessionText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TimingText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> InstrumentText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TowerText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StartLightsText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MessageText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MenuTitle;
	UPROPERTY(Transient) TArray<TObjectPtr<UButton>> MenuButtons;
};
