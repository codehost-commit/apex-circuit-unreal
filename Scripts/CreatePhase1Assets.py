import unreal


DATA_ASSET_PATH = "/Game/Data/DA_ApexFormulaCarTuning"
MAP_PATH = "/Game/Maps/L_ApexCircuit"


def log(message):
    unreal.log("APEX Phase 1: {}".format(message))


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def ensure_tuning_asset():
    ensure_directory("/Game/Data")
    if unreal.EditorAssetLibrary.does_asset_exist(DATA_ASSET_PATH):
        asset = unreal.EditorAssetLibrary.load_asset(DATA_ASSET_PATH)
        log("using existing tuning asset")
        return asset

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.ApexCarTuningDataAsset)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DA_ApexFormulaCarTuning",
        "/Game/Data",
        unreal.ApexCarTuningDataAsset,
        factory,
    )
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    log("created tuning asset")
    return asset


def spawn_environment():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 20000.0)
    )
    light.set_actor_label("Sun")
    light.set_actor_rotation(unreal.Rotator(-48.0, -32.0, 0.0), False)
    component = light.get_component_by_class(unreal.DirectionalLightComponent)
    component.set_editor_property("intensity", 8.0)

    skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(0.0, 0.0, 1000.0)
    )
    skylight.set_actor_label("Ambient Sky")
    sky_component = skylight.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_editor_property("intensity", 1.1)

    atmosphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0)
    )
    atmosphere.set_actor_label("Atmosphere")

    fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ExponentialHeightFog, unreal.Vector(0.0, 0.0, 0.0)
    )
    fog.set_actor_label("Distance Fog")


def ensure_map():
    ensure_directory("/Game/Maps")
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        log("using existing map")
        return

    if not unreal.EditorLevelLibrary.new_level(MAP_PATH):
        raise RuntimeError("Could not create {}".format(MAP_PATH))

    spawn_environment()
    track = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ApexTrackActor, unreal.Vector(0.0, 0.0, 0.0)
    )
    track.set_actor_label("APEX Circuit - Original Layout")
    track.build_track()
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Could not save {}".format(MAP_PATH))
    log("created playable map")


def main():
    ensure_tuning_asset()
    ensure_map()
    log("asset creation complete")


main()
