# APEX Circuit Unreal

This is the Unreal Engine 5.8 rebuild of the APEX Circuit Godot project. Phase 2
is playable: it loads the original circuit, presents an in-game UMG session
menu, spawns a deterministic physical opponent field, and runs practice,
qualifying, time trial, and race sessions.

## Local setup

- Engine: `/Users/rahul/UE_5.8`
- Editor: `/Users/rahul/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app`
- Platform: Apple Silicon macOS
- Source project: `../apex-circuit-godot`

Open `ApexCircuit.uproject` with Unreal Editor 5.8 and press Play. The opening
overlay offers `PRACTICE`, `QUALIFYING`, `TIME TRIAL`, and `RACE`. Race starts
use a five-light sequence; the others are live immediately. The project uses
Lumen, Virtual Shadow Maps, Nanite project support, and TSR defaults. On this
Mac, these are software/Metal rendering paths; DLSS is a later Windows/NVIDIA
integration, as planned.

The default editor and game map is `Content/Maps/L_ApexCircuit`. In the Unreal
Editor, press the Play button at the top of the viewport. The HUD will appear
once the player car is spawned.

| Input | Action |
| --- | --- |
| `W` | Throttle |
| `S` or `Space` | Brake |
| `A` / `D` | Steering |
| `Left Shift` | DRS when eligible |
| `E` | ERS deployment |
| `C` | Chase, T-cam, cockpit cameras |
| `R` | Return to the latest verified checkpoint |
| `Escape` | Pause/resume the current session |

## Phase 2 contents

- `Content/Data/ApexCircuitLayout.json`: versioned source of the exact 60-point
  Godot layout, gravel polygons, timing fractions, sectors, and DRS zones.
- `AApexTrackActor`: deterministic rounded-control-path port, 1.6 m sampling,
  4,239.6 m spline, road/kerb collision, grass and gravel surfaces.
- `AApexFormulaCar`: four-ray suspension, combined tyre forces, auto gearbox,
  aero, DRS, ERS, reset, and three cameras.
- `UApexCarTuningDataAsset`: editable Unreal mirror of the Godot car-tuning
  resource at `Content/Data/DA_ApexFormulaCarTuning`.
- `AApexRaceDirector`: session state, rolling grid, five lights, ordered
  checkpoint validation, sectors/laps, DRS, recovery, track-limit invalidation,
  and a live classification.
- `UApexSessionConfigDataAsset`: editable game rules at
  `Content/Data/DA_ApexSessionConfig`.
- `UApexAiDriverComponent`: fixed 30 Hz deterministic spline driver used by the
  seven-car physical opponent field. It is intentionally separate from the
  deferred learned-driver experiment.
- `UApexRaceHUDWidget`: native UMG menu, start lights, timing, live telemetry,
  DRS/ERS status, and a timing tower. It can become a designer-authored UMG
  Blueprint without changing race code.
- Layered low/mid/high RPM engine audio is imported to `Content/Audio/Engine`
  and spatially attached to each car. The original engineer recordings are
  imported to `Content/Audio/Engineer` for the radio presentation pass.
- `Scripts/ImportPhase2Assets.py`, `ValidatePhase2.py`, and `BuildProject.sh`
  reproduce the data asset, free-asset import, validation, and editor build.

## Reproduce Phase 2

```sh
./Scripts/BuildProject.sh
/Users/rahul/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd ApexCircuit.uproject \
  -run=pythonscript -script="$PWD/Scripts/ImportPhase2Assets.py" -unattended -nop4
```

All assets imported by the script are free/original inputs listed in
[`THIRD_PARTY_ASSETS.md`](THIRD_PARTY_ASSETS.md). The existing CC-BY branded
RB14 model is deliberately excluded from the Unreal default build.

To regenerate the Xcode project files from the terminal:

```sh
/Users/rahul/UE_5.8/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh \
  /Users/rahul/apex-circuit/apex-circuit-unreal/ApexCircuit.uproject
```

To build the editor target from the terminal:

```sh
/Users/rahul/UE_5.8/Engine/Build/BatchFiles/Mac/Build.sh \
  ApexCircuitEditor Mac Development \
  /Users/rahul/apex-circuit/apex-circuit-unreal/ApexCircuit.uproject
```

## Repository policy

Large Unreal assets use Git LFS. Before committing binary content, verify
`git lfs env` and use `git lfs status`. Do not commit `Binaries`, `DerivedDataCache`,
`Intermediate`, or `Saved`.

Generated binary maps, assets, textures, meshes, and audio are covered by
Git LFS attributes. The repository keeps source, config, and migration data
reviewable while Unreal stores editor assets in their native format.
