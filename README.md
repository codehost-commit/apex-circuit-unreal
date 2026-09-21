# APEX Circuit Unreal

This is the Unreal Engine 5.8 rebuild of the APEX Circuit Godot project. Phase 3
is playable and presentation-complete: it loads the original circuit, the
temporary high-detail RB14 development visual, a full physical opponent field,
PBR track surfaces, authored trackside composition, dynamic weather/time of day,
Lumen lighting, VSM shadows, Niagara vehicle effects, and the complete session UI.

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
| `P` | Freeze/unfreeze photo mode; orbit with the existing camera controls |
| `F8` | Replay the latest eight seconds from the chase camera |
| `B` / `T` | Toggle ABS / traction control |
| `M` | Cycle balanced, overtake, and harvest ERS strategies |
| Gamepad triggers / left stick | Throttle, brake, steering |

## Phase 3 contents

- The Lumen warning is fixed at its source: software Lumen now has mesh distance
  fields enabled and uses the supported Metal/SM6 path on macOS. Hardware Lumen
  remains available as a future Windows/RTX path rather than being falsely
  enabled on this Mac.
- The 17 MB source RB14 imports into a full native static mesh plus its authored
  material/texture set. It replaces the cube proxy for the player and AI field.
- Asphalt, kerb, grass, and gravel use PBR material graphs with linear roughness,
  DirectX-corrected normals, specular response, and a shared runtime weather
  parameter collection. Dedicated carbon, tyre, brake-glow, rain-light, glass,
  display, barrier, and screen materials cover the car and circuit vocabulary.
  The authoritative gravel zones now have rendered geometry.
- A deterministic presentation director builds Armco, catch fencing, pit and
  paddock structures, tiered grandstands, marshal/lighting infrastructure,
  floodlights, pit wall, marshal posts, track cameras, CC0 Nanite vegetation,
  volumetric fog/clouds, sky atmosphere, physical exposure, depth of field,
  lens flare/dirt, filmic color response, and a late-afternoon lighting rig.
- Rain and wetness are linked across the world, track and vehicle effects. The
  car uses packaged Niagara spray/lockup systems; the camera-local rain field is
  instanced for predictable performance on Apple Silicon.
- Fuel use, tyre temperature/wear, impact damage, switchable ABS, active traction
  intervention, three ERS strategies, replay capture, photo freeze, and controller
  mappings extend the gameplay layer. The HUD exposes live condition, assists,
  strategy, and weather telemetry; the opening overlay includes assist controls.
- `DefaultScalability.ini` provides a High 60 FPS target and a separate
  Cinematic capture tier. TSR/dynamic resolution is the cross-platform default;
  DLSS remains intentionally deferred to the Windows/NVIDIA plugin path.

The RB14 is **CC BY 4.0, not CC0**. Its exact required attribution and the CC0
nature sources are documented in [`THIRD_PARTY_ASSETS.md`](THIRD_PARTY_ASSETS.md).
The branded RB14 remains a temporary development asset and must be replaced by
the original unbranded APEX Formula body before public release.

## Reproduce and validate Phase 3

```sh
./Scripts/BuildProject.sh
/Users/rahul/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd "$PWD/ApexCircuit.uproject" \
  -run=pythonscript -script="$PWD/Scripts/ImportPhase3Assets.py" -unattended -nop4
/Users/rahul/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd "$PWD/ApexCircuit.uproject" \
  -run=pythonscript -script="$PWD/Scripts/ValidatePhase3.py" -unattended -nop4
```

`./Scripts/CapturePhase3.sh` launches a 1440p cinematic-quality capture session.
Rendering targets and profiling commands are documented in
[`PERFORMANCE_BUDGETS.md`](PERFORMANCE_BUDGETS.md).

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

All assets imported by the script are free/original inputs or the explicitly
requested attributed RB14 listed in [`THIRD_PARTY_ASSETS.md`](THIRD_PARTY_ASSETS.md).

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
