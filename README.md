# APEX Circuit Unreal

This is the Unreal Engine 5.8 rebuild of the APEX Circuit Godot project.
The project is intentionally code-first during the foundation stage so the
track data, vehicle physics, race rules, telemetry, and tools remain reviewable
in source control.

## Local setup

- Engine: `/Users/rahul/UE_5.8`
- Editor: `/Users/rahul/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app`
- Platform: Apple Silicon macOS
- Source project: `../apex-circuit-godot`

Open `ApexCircuit.uproject` with Unreal Editor 5.8. The first launch may
compile the C++ module and create generated folders that are intentionally
ignored by Git.

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

Phase 1 implementation will add the authoritative circuit data import, the
first playable formula car, and a minimal test map on top of this foundation.
