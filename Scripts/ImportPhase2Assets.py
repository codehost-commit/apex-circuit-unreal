"""Repeatable Phase 2 asset import. Only free/original sources listed in THIRD_PARTY_ASSETS.md are included."""

import os
import unreal


ROOT = unreal.Paths.project_content_dir() + "SourceAssets/"


def log(message):
    unreal.log("APEX Phase 2 import: {}".format(message))


def import_files(source_dir, destination, extensions):
    if not os.path.isdir(source_dir):
        raise RuntimeError("Missing source directory: {}".format(source_dir))
    files = [
        os.path.join(source_dir, name)
        for name in sorted(os.listdir(source_dir))
        if os.path.splitext(name)[1].lower() in extensions
    ]
    if not files:
        return
    tasks = []
    for filename in files:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", filename)
        task.set_editor_property("destination_path", destination)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    log("imported {} file(s) into {}".format(len(files), destination))


def ensure_session_config():
    path = "/Game/Data/DA_ApexSessionConfig"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.ApexSessionConfigDataAsset)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DA_ApexSessionConfig", "/Game/Data", unreal.ApexSessionConfigDataAsset, factory
    )
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    log("created session config asset")


def main():
    ensure_session_config()
    import_files(ROOT + "Textures", "/Game/Art/Textures", {".png", ".jpg", ".jpeg"})
    import_files(ROOT + "HDRI", "/Game/Art/HDRI", {".hdr"})
    import_files(ROOT + "UI", "/Game/Art/UI", {".png"})
    import_files(ROOT + "Branding", "/Game/Art/Branding", {".png"})
    import_files(ROOT + "Trackside", "/Game/Art/Trackside", {".png"})
    import_files(ROOT + "Sponsors", "/Game/Art/Sponsors", {".png"})
    import_files(ROOT + "Audio", "/Game/Audio/Engine", {".wav"})
    import_files(ROOT + "Audio/Engineer", "/Game/Audio/Engineer", {".mp3"})
    unreal.EditorAssetLibrary.save_directory("/Game", False, True)
    log("Phase 2 asset import complete")


main()
