# APEX Circuit Unreal

This is the Unreal Engine 5.8 rebuild of the APEX Circuit Godot project. Phase 1
is playable: opening the project loads the original circuit map and pressing
Play spawns one Formula car on the exact start/finish seam.

## Local setup

- Engine: `/Users/rahul/UE_5.8`
- Editor: `/Users/rahul/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app`
- Platform: Apple Silicon macOS
- Source project: `../apex-circuit-godot`

Open `ApexCircuit.uproject` with Unreal Editor 5.8. The first launch may
compile the C++ module and create generated folders that are intentionally
ignored by Git.

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
| `R` | Return to latest timing checkpoint |
| `Escape` | Pause |

## Phase 1 contents

- `Content/Data/ApexCircuitLayout.json`: versioned source of the exact 60-point
  Godot layout, gravel polygons, timing fractions, sectors, and DRS zones.
- `AApexTrackActor`: deterministic rounded-control-path port, 1.6 m sampling,
  4,239.6 m spline, road/kerb collision, grass and gravel surfaces.
- `AApexFormulaCar`: four-ray suspension, combined tyre forces, auto gearbox,
  aero, DRS, ERS, reset, and three cameras.
- `UApexCarTuningDataAsset`: editable Unreal mirror of the Godot car-tuning
  resource at `Content/Data/DA_ApexFormulaCarTuning`.
- `AApexRaceDirector` and `UApexTelemetryComponent`: lap/sectors/checkpoints,
  DRS eligibility, debug HUD, and sampled vehicle telemetry.
- `Scripts/CreatePhase1Assets.py` and `Scripts/ValidatePhase1.py`: repeatable
  editor asset creation and presence checks.

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
