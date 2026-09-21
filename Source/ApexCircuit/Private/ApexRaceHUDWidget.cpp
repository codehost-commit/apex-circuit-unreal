#include "ApexRaceHUDWidget.h"

#include "ApexCircuitGameMode.h"
#include "ApexFormulaCar.h"
#include "ApexRaceDirector.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	UTextBlock* AddText(UWidgetTree* Tree, UCanvasPanel* Root, const FString& Text, float Size, FVector2D Position, FVector2D PanelSize, const FLinearColor& Color)
	{
		UTextBlock* Label = Tree->ConstructWidget<UTextBlock>(); Label->SetText(FText::FromString(Text)); Label->SetFont(FSlateFontInfo(Label->GetFont().FontObject, Size)); Label->SetColorAndOpacity(FSlateColor(Color));
		UCanvasPanelSlot* Slot = Root->AddChildToCanvas(Label); Slot->SetPosition(Position); Slot->SetSize(PanelSize); return Label;
	}
	UButton* AddButton(UWidgetTree* Tree, UCanvasPanel* Root, const FString& Label, FVector2D Position)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(); UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(); Text->SetText(FText::FromString(Label)); Text->SetJustification(ETextJustify::Center); Text->SetFont(FSlateFontInfo(Text->GetFont().FontObject, 20)); Button->AddChild(Text);
		UCanvasPanelSlot* Slot = Root->AddChildToCanvas(Button); Slot->SetPosition(Position); Slot->SetSize(FVector2D(275.0f, 46.0f)); return Button;
	}
	FString TimeText(float Seconds) { return Seconds <= 0.0f ? TEXT("--:--.---") : FString::Printf(TEXT("%02d:%06.3f"), FMath::FloorToInt(Seconds / 60.0f), FMath::Fmod(Seconds, 60.0f)); }
}

void UApexRaceHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ApexRaceRoot")); WidgetTree->RootWidget = Root;
	SessionText = AddText(WidgetTree, Root, TEXT("APEX CIRCUIT"), 22, FVector2D(28, 24), FVector2D(530, 42), FLinearColor(0.75f, 0.9f, 1.0f));
	TimingText = AddText(WidgetTree, Root, TEXT(""), 18, FVector2D(28, 70), FVector2D(370, 150), FLinearColor::White);
	InstrumentText = AddText(WidgetTree, Root, TEXT(""), 28, FVector2D(28, 790), FVector2D(530, 80), FLinearColor::White);
	TowerText = AddText(WidgetTree, Root, TEXT(""), 17, FVector2D(1450, 42), FVector2D(360, 290), FLinearColor::White);
	StartLightsText = AddText(WidgetTree, Root, TEXT(""), 42, FVector2D(800, 80), FVector2D(350, 70), FLinearColor(1.0f, 0.1f, 0.08f));
	MessageText = AddText(WidgetTree, Root, TEXT("SELECT A SESSION"), 22, FVector2D(720, 880), FVector2D(520, 50), FLinearColor(0.2f, 0.85f, 1.0f));
	MenuTitle = AddText(WidgetTree, Root, TEXT("APEX CIRCUIT\nFORMULA RACING"), 36, FVector2D(70, 280), FVector2D(500, 130), FLinearColor::White);
	UButton* Practice = AddButton(WidgetTree, Root, TEXT("PRACTICE"), FVector2D(70, 440)); Practice->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::StartPractice);
	UButton* Qualifying = AddButton(WidgetTree, Root, TEXT("QUALIFYING"), FVector2D(70, 494)); Qualifying->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::StartQualifying);
	UButton* Trial = AddButton(WidgetTree, Root, TEXT("TIME TRIAL"), FVector2D(70, 548)); Trial->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::StartTimeTrial);
	UButton* Race = AddButton(WidgetTree, Root, TEXT("RACE"), FVector2D(70, 602)); Race->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::StartRace);
	UButton* RestartButton = AddButton(WidgetTree, Root, TEXT("RESTART"), FVector2D(70, 680)); RestartButton->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::Restart);
	UButton* RecoverButton = AddButton(WidgetTree, Root, TEXT("RECOVER CAR"), FVector2D(70, 734)); RecoverButton->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::Recover);
	UButton* PauseButton = AddButton(WidgetTree, Root, TEXT("PAUSE"), FVector2D(70, 788)); PauseButton->OnClicked.AddDynamic(this, &UApexRaceHUDWidget::TogglePause);
	MenuButtons = {Practice, Qualifying, Trial, Race};
}

void UApexRaceHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	AApexCircuitGameMode* Mode = Cast<AApexCircuitGameMode>(UGameplayStatics::GetGameMode(this)); if (!Mode || !Mode->GetRaceDirector() || !Mode->GetPlayerCar()) return;
	AApexRaceDirector* Director = Mode->GetRaceDirector(); AApexFormulaCar* Car = Mode->GetPlayerCar();
	SetText(SessionText, FString::Printf(TEXT("APEX CIRCUIT  |  %s  |  LAP %d"), *UEnum::GetValueAsString(Director->GetSessionMode()).RightChop(18), Director->GetCurrentLap()));
	SetText(TimingText, FString::Printf(TEXT("LAP  %s%s\nLAST %s\nDRS  %s\nERS  %.1f MJ"), *TimeText(Director->GetCurrentLapTimeSeconds()), Director->IsCurrentLapValid() ? TEXT("") : TEXT("  INVALID"), *TimeText(Director->GetLastLapTimeSeconds()), Car->IsDrsAvailable() ? TEXT("READY") : TEXT("CLOSED"), Car->GetEnergyMj()));
	SetText(InstrumentText, FString::Printf(TEXT("%03d KM/H    GEAR %d    %05d RPM"), FMath::RoundToInt(Car->GetSpeedKph()), Car->GetGear(), FMath::RoundToInt(Car->GetEngineRpm())));
	FString Tower; for (const FApexClassificationRow& Row : Director->GetClassification()) Tower += FString::Printf(TEXT("P%02d  %-12s  %s\n"), Row.Position, *Row.DriverName, Row.Position == 1 ? TEXT("LEADER") : *FString::Printf(TEXT("+%.1f"), Row.GapSeconds)); SetText(TowerText, Tower);
	SetText(StartLightsText, Director->GetStartLights() ? FString::ChrN(Director->GetStartLights(), 'O') : TEXT("")); SetText(MessageText, Director->GetRaceMessage());
	const bool bMenu = Director->GetSessionState() == EApexSessionState::Menu; MenuTitle->SetVisibility(bMenu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed); for (UButton* Button : MenuButtons) if (Button) Button->SetVisibility(bMenu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UApexRaceHUDWidget::SetText(UTextBlock* Target, const FString& Value) const { if (Target) Target->SetText(FText::FromString(Value)); }
void UApexRaceHUDWidget::StartMode(uint8 Mode) { if (AApexCircuitGameMode* GameMode = Cast<AApexCircuitGameMode>(UGameplayStatics::GetGameMode(this))) if (AApexRaceDirector* Director = GameMode->GetRaceDirector()) Director->StartSession(static_cast<EApexSessionMode>(Mode)); }
void UApexRaceHUDWidget::StartPractice() { StartMode(static_cast<uint8>(EApexSessionMode::Practice)); }
void UApexRaceHUDWidget::StartQualifying() { StartMode(static_cast<uint8>(EApexSessionMode::Qualifying)); }
void UApexRaceHUDWidget::StartTimeTrial() { StartMode(static_cast<uint8>(EApexSessionMode::TimeTrial)); }
void UApexRaceHUDWidget::StartRace() { StartMode(static_cast<uint8>(EApexSessionMode::Race)); }
void UApexRaceHUDWidget::Restart() { if (AApexCircuitGameMode* GameMode = Cast<AApexCircuitGameMode>(UGameplayStatics::GetGameMode(this))) if (AApexRaceDirector* Director = GameMode->GetRaceDirector()) Director->RestartSession(); }
void UApexRaceHUDWidget::Recover() { if (AApexCircuitGameMode* GameMode = Cast<AApexCircuitGameMode>(UGameplayStatics::GetGameMode(this))) if (AApexRaceDirector* Director = GameMode->GetRaceDirector()) Director->RecoverPlayer(); }
void UApexRaceHUDWidget::TogglePause() { if (AApexCircuitGameMode* GameMode = Cast<AApexCircuitGameMode>(UGameplayStatics::GetGameMode(this))) if (AApexRaceDirector* Director = GameMode->GetRaceDirector()) Director->TogglePause(); }
