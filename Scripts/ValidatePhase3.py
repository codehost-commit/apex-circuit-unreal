"""Editor-side validation for the complete Phase 3 presentation layer."""

import os
import unreal


def fail(message):
    unreal.log_error("APEX Phase 3 validation: {}".format(message))
    raise RuntimeError(message)


required_assets = [
    "/Game/Vehicles/RB14/rb14/StaticMeshes/SM_RB14",
    "/Game/Art/Materials/M_ApexAsphalt",
    "/Game/Art/Materials/M_ApexGrass",
    "/Game/Art/Materials/M_ApexGravel",
    "/Game/Art/Materials/M_ApexKerb",
    "/Game/Art/Materials/M_ApexBarrier",
    "/Game/Art/Materials/M_ApexTyre",
    "/Game/Art/Materials/M_ApexBrakeGlow",
    "/Game/Art/Materials/M_ApexRainLight",
    "/Game/Art/Materials/M_ApexGlass",
    "/Game/Art/Materials/M_ApexDisplay",
    "/Game/Art/Materials/MPC_ApexWeather",
    "/Game/Art/Materials/M_ApexLight",
    "/Game/Art/Materials/M_ApexSponsor",
    "/Game/FX/NS_ApexWheelSpray",
    "/Game/FX/NS_ApexTyreBurst",
    "/Game/Environment/Nature/IslandTree/island_tree/StaticMeshes/SM_IslandTree",
    "/Game/Environment/Nature/PineSaplings/pine_saplings/StaticMeshes/pine_sapling_small_a",
]

for path in required_assets:
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        fail("missing required Phase 3 asset: {}".format(path))

weather = unreal.EditorAssetLibrary.load_asset("/Game/Art/Materials/MPC_ApexWeather")
weather_names = {
    str(parameter.get_editor_property("parameter_name"))
    for parameter in weather.get_editor_property("scalar_parameters")
}
if not {"Wetness", "RainIntensity"}.issubset(weather_names):
    fail("weather parameter collection does not expose wetness and rain")

mesh = unreal.EditorAssetLibrary.load_asset(required_assets[0])
box = mesh.get_bounding_box()
size = box.max - box.min
if max(size.x, size.y, size.z) < 450.0:
    fail("RB14 mesh is not imported at full vehicle scale: {}".format(size))

engine_ini = os.path.join(unreal.Paths.project_config_dir(), "DefaultEngine.ini")
with open(engine_ini, "r", encoding="utf-8") as config_file:
    config = config_file.read()
if "r.GenerateMeshDistanceFields=True" not in config:
    fail("software Lumen requires Generate Mesh Distance Fields")
if "r.Lumen.TraceMeshSDFs=1" not in config:
    fail("software Lumen mesh SDF tracing is not enabled")

windows_ini = os.path.join(
    unreal.Paths.project_config_dir(), "Windows", "WindowsEngine.ini"
)
with open(windows_ini, "r", encoding="utf-8") as config_file:
    windows_config = config_file.read()
if "r.Lumen.HardwareRayTracing=1" not in windows_config:
    fail("supported Windows GPUs have no hardware Lumen path")

scalability_ini = os.path.join(unreal.Paths.project_config_dir(), "DefaultScalability.ini")
with open(scalability_ini, "r", encoding="utf-8") as config_file:
    scalability = config_file.read()
if "[GlobalIlluminationQuality@1]" not in scalability:
    fail("playable laptop scalability tier is missing")
if "[GlobalIlluminationQuality@Cine]" not in scalability:
    fail("cinematic scalability tier is missing")

unreal.log(
    "APEX Phase 3 validation: RB14 {}, PBR materials, Niagara effects, "
    "software Lumen distance fields and scalability tiers are present.".format(size)
)
