"""Build the Phase 3 visual asset layer from reproducible source files.

The script deliberately keeps editor-authored binary assets reproducible. It imports
the temporary CC BY 4.0 RB14 development model and creates PBR master materials that
are consumed by the procedural track at runtime.
"""

import os
import unreal


CONTENT = unreal.Paths.project_content_dir()
RB14_SOURCE = os.path.join(CONTENT, "SourceAssets/Vehicles/RB14/rb14.glb")
WEATHER_COLLECTION = "/Game/Art/Materials/MPC_ApexWeather"


def log(message):
    unreal.log("APEX Phase 3: {}".format(message))


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError("Missing required asset: {}".format(path))
    return asset


def import_rb14():
    if not os.path.isfile(RB14_SOURCE):
        raise RuntimeError("Missing RB14 source: {}".format(RB14_SOURCE))
    ensure_directory("/Game/Vehicles/RB14")
    if unreal.EditorAssetLibrary.does_asset_exist(
        "/Game/Vehicles/RB14/rb14/StaticMeshes/SM_RB14"
    ):
        log("using existing RB14 import")
        return
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", RB14_SOURCE)
    task.set_editor_property("destination_path", "/Game/Vehicles/RB14")
    task.set_editor_property("destination_name", "SM_RB14")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    log("RB14 import paths: {}".format(list(task.get_editor_property("imported_object_paths"))))


def import_nature():
    sources = [
        ("island_tree.glb", "IslandTree", "SM_IslandTree", "/Game/Environment/Nature/IslandTree/island_tree/StaticMeshes/SM_IslandTree"),
        ("pine_saplings.glb", "PineSaplings", "SM_PineSaplings", "/Game/Environment/Nature/PineSaplings/pine_saplings/StaticMeshes/pine_sapling_small_a"),
    ]
    ensure_directory("/Game/Environment/Nature")
    for filename, folder, asset_name, expected in sources:
        if unreal.EditorAssetLibrary.does_asset_exist(expected):
            continue
        source = os.path.join(CONTENT, "SourceAssets/Nature", filename)
        if not os.path.isfile(source):
            raise RuntimeError("Missing nature source: {}".format(source))
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", source)
        task.set_editor_property("destination_path", "/Game/Environment/Nature/{}".format(folder))
        task.set_editor_property("destination_name", asset_name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        log("{} import paths: {}".format(folder, list(task.get_editor_property("imported_object_paths"))))


def enable_nanite():
    meshes = [
        "/Game/Vehicles/RB14/rb14/StaticMeshes/SM_RB14",
        "/Game/Environment/Nature/IslandTree/island_tree/StaticMeshes/SM_IslandTree",
        "/Game/Environment/Nature/PineSaplings/pine_saplings/StaticMeshes/pine_sapling_small_a",
        "/Game/Environment/Nature/PineSaplings/pine_saplings/StaticMeshes/pine_sapling_small_b",
        "/Game/Environment/Nature/PineSaplings/pine_saplings/StaticMeshes/pine_sapling_small_c",
    ]
    for path in meshes:
        mesh = unreal.EditorAssetLibrary.load_asset(path)
        if mesh is None:
            raise RuntimeError("Missing Nanite candidate: {}".format(path))
        settings = mesh.get_editor_property("nanite_settings")
        settings.set_editor_property("enabled", True)
        settings.set_editor_property("fallback_percent_triangles", 1.0)
        mesh.set_editor_property("nanite_settings", settings)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    log("enabled Nanite on imported showcase meshes")


def make_material(name):
    path = "/Game/Art/Materials/{}".format(name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    return unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew()
    )


def create_weather_collection():
    if unreal.EditorAssetLibrary.does_asset_exist(WEATHER_COLLECTION):
        unreal.EditorAssetLibrary.delete_asset(WEATHER_COLLECTION)
    collection = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "MPC_ApexWeather",
        "/Game/Art/Materials",
        unreal.MaterialParameterCollection,
        unreal.MaterialParameterCollectionFactoryNew(),
    )
    wetness = unreal.CollectionScalarParameter()
    wetness.set_editor_property("parameter_name", "Wetness")
    wetness.set_editor_property("default_value", 0.0)
    rain = unreal.CollectionScalarParameter()
    rain.set_editor_property("parameter_name", "RainIntensity")
    rain.set_editor_property("default_value", 0.0)
    collection.set_editor_property("scalar_parameters", [wetness, rain])
    unreal.EditorAssetLibrary.save_loaded_asset(collection)
    return collection


def expr(material, expression_class, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(
        material, expression_class, x, y
    )


def scalar(material, name, default, x, y):
    node = expr(material, unreal.MaterialExpressionScalarParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def collection_scalar(material, name, x, y):
    node = expr(material, unreal.MaterialExpressionCollectionParameter, x, y)
    node.set_editor_property("collection", load(WEATHER_COLLECTION))
    node.set_editor_property("parameter_name", name)
    return node


def vector(material, name, default, x, y):
    node = expr(material, unreal.MaterialExpressionVectorParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def texture_sample(material, texture_path, x, y, sampler_type=None):
    node = expr(material, unreal.MaterialExpressionTextureSample, x, y)
    node.set_editor_property("texture", load(texture_path))
    if sampler_type is not None:
        node.set_editor_property("sampler_type", sampler_type)
    return node


def connect(source, output_name, target, input_name):
    unreal.MaterialEditingLibrary.connect_material_expressions(
        source, output_name, target, input_name
    )


def connect_property(source, output_name, material, prop):
    unreal.MaterialEditingLibrary.connect_material_property(
        source, output_name, prop
    )


def create_surface_material(name, diffuse_path, normal_path, roughness_path, tint):
    material = make_material(name)
    uv = expr(material, unreal.MaterialExpressionTextureCoordinate, -1250, -100)
    uv.set_editor_property("u_tiling", 0.18)
    uv.set_editor_property("v_tiling", 0.18)
    albedo = texture_sample(material, diffuse_path, -1000, -300)
    normal = texture_sample(
        material, normal_path, -1000, 40, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    )
    rough = texture_sample(
        material, roughness_path, -1000, 350, unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR
    )
    for sample in (albedo, normal, rough):
        connect(uv, "", sample, "UVs")

    tint_node = vector(material, "SurfaceTint", tint, -1000, -520)
    tint_multiply = expr(material, unreal.MaterialExpressionMultiply, -720, -300)
    connect(albedo, "RGB", tint_multiply, "A")
    connect(tint_node, "", tint_multiply, "B")

    wetness = collection_scalar(material, "Wetness", -720, 550)
    wet_dark = vector(material, "WetDarkening", unreal.LinearColor(0.18, 0.21, 0.24, 1.0), -520, -500)
    base_lerp = expr(material, unreal.MaterialExpressionLinearInterpolate, -260, -280)
    connect(tint_multiply, "", base_lerp, "A")
    connect(wet_dark, "", base_lerp, "B")
    connect(wetness, "", base_lerp, "Alpha")
    connect_property(base_lerp, "", material, unreal.MaterialProperty.MP_BASE_COLOR)

    wet_roughness = scalar(material, "WetRoughness", 0.08, -500, 360)
    rough_lerp = expr(material, unreal.MaterialExpressionLinearInterpolate, -220, 250)
    connect(rough, "R", rough_lerp, "A")
    connect(wet_roughness, "", rough_lerp, "B")
    connect(wetness, "", rough_lerp, "Alpha")
    connect_property(rough_lerp, "", material, unreal.MaterialProperty.MP_ROUGHNESS)
    connect_property(normal, "RGB", material, unreal.MaterialProperty.MP_NORMAL)

    specular = scalar(material, "Specular", 0.5, -200, 480)
    connect_property(specular, "", material, unreal.MaterialProperty.MP_SPECULAR)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def configure_pbr_textures():
    normal_paths = [
        "/Game/Art/Textures/asphalt_track_nor_gl",
        "/Game/Art/Textures/grass_ground_nor_gl",
        "/Game/Art/Textures/gravel_floor_nor_gl",
    ]
    roughness_paths = [
        "/Game/Art/Textures/asphalt_track_rough",
        "/Game/Art/Textures/grass_ground_rough",
        "/Game/Art/Textures/gravel_floor_rough",
        "/Game/Art/Textures/wet_asphalt_microdetail",
    ]
    for path in normal_paths:
        texture = load(path)
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("flip_green_channel", True)
        texture.set_editor_property(
            "compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP
        )
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
    for path in roughness_paths:
        texture = load(path)
        texture.set_editor_property("srgb", False)
        texture.set_editor_property(
            "compression_settings", unreal.TextureCompressionSettings.TC_MASKS
        )
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
    log("configured linear roughness and DirectX normal-map imports")


def create_kerb_material():
    material = make_material("M_ApexKerb")
    vertex = expr(material, unreal.MaterialExpressionVertexColor, -700, -100)
    wetness = collection_scalar(material, "Wetness", -700, 220)
    wet_color = vector(material, "WetDarkening", unreal.LinearColor(0.18, 0.18, 0.2, 1), -700, 30)
    color_lerp = expr(material, unreal.MaterialExpressionLinearInterpolate, -350, -80)
    connect(vertex, "RGB", color_lerp, "A")
    connect(wet_color, "", color_lerp, "B")
    connect(wetness, "", color_lerp, "Alpha")
    connect_property(color_lerp, "", material, unreal.MaterialProperty.MP_BASE_COLOR)
    dry_rough = scalar(material, "DryRoughness", 0.62, -350, 220)
    wet_rough = scalar(material, "WetRoughness", 0.1, -350, 320)
    rough_lerp = expr(material, unreal.MaterialExpressionLinearInterpolate, -80, 230)
    connect(dry_rough, "", rough_lerp, "A")
    connect(wet_rough, "", rough_lerp, "B")
    connect(wetness, "", rough_lerp, "Alpha")
    connect_property(rough_lerp, "", material, unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def create_simple_material(name, color, roughness, metallic=0.0, emissive=None):
    material = make_material(name)
    base = vector(material, "BaseColor", color, -400, -80)
    rough = scalar(material, "Roughness", roughness, -400, 100)
    metal = scalar(material, "Metallic", metallic, -400, 190)
    connect_property(base, "", material, unreal.MaterialProperty.MP_BASE_COLOR)
    connect_property(rough, "", material, unreal.MaterialProperty.MP_ROUGHNESS)
    connect_property(metal, "", material, unreal.MaterialProperty.MP_METALLIC)
    if emissive is not None:
        emission = vector(material, "Emission", emissive, -400, 300)
        strength = scalar(material, "EmissionStrength", 25.0, -400, 410)
        multiply = expr(material, unreal.MaterialExpressionMultiply, -120, 320)
        connect(emission, "", multiply, "A")
        connect(strength, "", multiply, "B")
        connect_property(multiply, "", material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def create_glass_material():
    material = make_material("M_ApexGlass")
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    base = vector(material, "GlassTint", unreal.LinearColor(0.04, 0.09, 0.13, 1), -450, -100)
    opacity = scalar(material, "Opacity", 0.22, -450, 40)
    rough = scalar(material, "Roughness", 0.08, -450, 160)
    connect_property(base, "", material, unreal.MaterialProperty.MP_BASE_COLOR)
    connect_property(opacity, "", material, unreal.MaterialProperty.MP_OPACITY)
    connect_property(rough, "", material, unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def create_sponsor_material():
    material = make_material("M_ApexSponsor")
    sample = texture_sample(material, "/Game/Art/Sponsors/atlas", -520, -80)
    connect_property(sample, "RGB", material, unreal.MaterialProperty.MP_BASE_COLOR)
    strength = scalar(material, "ScreenBrightness", 0.65, -500, 180)
    emission = expr(material, unreal.MaterialExpressionMultiply, -220, 20)
    connect(sample, "RGB", emission, "A")
    connect(strength, "", emission, "B")
    connect_property(emission, "", material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    rough = scalar(material, "Roughness", 0.28, -220, 210)
    connect_property(rough, "", material, unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def create_materials():
    ensure_directory("/Game/Art/Materials")
    create_weather_collection()
    create_surface_material(
        "M_ApexAsphalt",
        "/Game/Art/Textures/asphalt_track_diffuse",
        "/Game/Art/Textures/asphalt_track_nor_gl",
        "/Game/Art/Textures/asphalt_track_rough",
        unreal.LinearColor(0.72, 0.73, 0.75, 1.0),
    )
    create_surface_material(
        "M_ApexGrass",
        "/Game/Art/Textures/grass_ground_diffuse",
        "/Game/Art/Textures/grass_ground_nor_gl",
        "/Game/Art/Textures/grass_ground_rough",
        unreal.LinearColor(0.46, 0.62, 0.38, 1.0),
    )
    create_surface_material(
        "M_ApexGravel",
        "/Game/Art/Textures/gravel_floor_diffuse",
        "/Game/Art/Textures/gravel_floor_nor_gl",
        "/Game/Art/Textures/gravel_floor_rough",
        unreal.LinearColor(0.72, 0.67, 0.58, 1.0),
    )
    create_kerb_material()
    create_simple_material("M_ApexCarbon", unreal.LinearColor(0.006, 0.008, 0.012, 1), 0.23, 0.2)

    create_simple_material("M_ApexTyre", unreal.LinearColor(0.008, 0.009, 0.011, 1), 0.67, 0.0)
    create_simple_material("M_ApexBrakeGlow", unreal.LinearColor(0.18, 0.012, 0.002, 1), 0.34, 0.72, unreal.LinearColor(1.0, 0.025, 0.002, 1))
    create_simple_material("M_ApexRainLight", unreal.LinearColor(0.18, 0.0, 0.0, 1), 0.16, 0.2, unreal.LinearColor(1.0, 0.0, 0.0, 1))
    create_simple_material("M_ApexDisplay", unreal.LinearColor(0.008, 0.02, 0.028, 1), 0.2, 0.18, unreal.LinearColor(0.08, 0.72, 1.0, 1))
    create_glass_material()
    create_simple_material("M_ApexBarrier", unreal.LinearColor(0.14, 0.16, 0.18, 1), 0.34, 0.72)
    create_simple_material("M_ApexStructure", unreal.LinearColor(0.025, 0.035, 0.045, 1), 0.45, 0.18)
    create_simple_material("M_ApexLight", unreal.LinearColor(0.8, 0.86, 0.92, 1), 0.15, 0.45, unreal.LinearColor(1.0, 0.83, 0.58, 1))
    create_sponsor_material()
    log("created Phase 3 PBR materials")


def create_niagara_fx():
    """Clone engine-owned Niagara templates so effects are packaged with the game."""
    ensure_directory("/Game/FX")
    templates = {
        "/Niagara/DefaultAssets/Templates/Systems/FountainLightweight": "/Game/FX/NS_ApexWheelSpray",
        "/Niagara/DefaultAssets/Templates/Systems/DirectionalBurstLightweight": "/Game/FX/NS_ApexTyreBurst",
    }
    for source, destination in templates.items():
        if unreal.EditorAssetLibrary.does_asset_exist(destination):
            unreal.EditorAssetLibrary.delete_asset(destination)
        source_asset = unreal.load_asset(source)
        destination_path, destination_name = destination.rsplit("/", 1)
        duplicated = None
        if source_asset is not None:
            duplicated = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
                destination_name, destination_path, source_asset
            )
        if duplicated is None:
            raise RuntimeError("Could not create Niagara effect from {}".format(source))
    unreal.EditorAssetLibrary.save_directory("/Game/FX", False, True)
    log("created packaged Niagara spray and burst systems")


def main():
    import_rb14()
    import_nature()
    enable_nanite()
    configure_pbr_textures()
    create_materials()
    create_niagara_fx()
    unreal.EditorAssetLibrary.save_directory("/Game/Art/Materials", False, True)
    unreal.EditorAssetLibrary.save_directory("/Game/Vehicles/RB14", False, True)
    log("Phase 3 asset import complete")


main()
