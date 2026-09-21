import os

import unreal


MAP_PATH = "/Game/Maps/L_ApexCircuit"
TUNING_PATH = "/Game/Data/DA_ApexFormulaCarTuning"


def fail(message):
    unreal.log_error("APEX Phase 1 validation: {}".format(message))
    raise RuntimeError(message)


if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    fail("playable map is missing")

if not unreal.EditorAssetLibrary.does_asset_exist(TUNING_PATH):
    fail("formula tuning asset is missing")

layout_path = unreal.Paths.project_content_dir() + "Data/ApexCircuitLayout.json"
if not os.path.isfile(layout_path):
    fail("authoritative circuit JSON is missing")

unreal.log("APEX Phase 1 validation: map, tuning asset, and circuit source data are present.")
