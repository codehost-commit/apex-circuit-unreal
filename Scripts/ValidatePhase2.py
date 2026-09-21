import os
import unreal


def fail(message):
    unreal.log_error("APEX Phase 2 validation: {}".format(message))
    raise RuntimeError(message)


required = [
    "/Game/Maps/L_ApexCircuit",
    "/Game/Data/DA_ApexFormulaCarTuning",
    "/Game/Data/DA_ApexSessionConfig",
    "/Game/Audio/Engine/f1_engine_low",
    "/Game/Audio/Engine/f1_engine_mid",
    "/Game/Audio/Engine/f1_engine_high",
]
for path in required:
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        fail("missing required asset: {}".format(path))

layout_path = unreal.Paths.project_content_dir() + "Data/ApexCircuitLayout.json"
if not os.path.isfile(layout_path):
    fail("authoritative circuit JSON is missing")

unreal.log("APEX Phase 2 validation: session configuration, imported free assets, and gameplay data are present.")
