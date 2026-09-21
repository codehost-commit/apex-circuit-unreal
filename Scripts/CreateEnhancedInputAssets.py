import unreal


INPUT_DIRECTORY = "/Game/Input"


def ensure_action(name, value_type):
    path = INPUT_DIRECTORY + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)

    factory = unreal.InputAction_Factory()
    factory.set_editor_property("input_action_class", unreal.InputAction)
    action = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, INPUT_DIRECTORY, unreal.InputAction, factory
    )
    action.set_editor_property("value_type", value_type)
    unreal.EditorAssetLibrary.save_loaded_asset(action)
    return action


def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(INPUT_DIRECTORY):
        unreal.EditorAssetLibrary.make_directory(INPUT_DIRECTORY)

    axis = unreal.InputActionValueType.AXIS1D
    boolean = unreal.InputActionValueType.BOOLEAN
    actions = {
        "IA_Throttle": ensure_action("IA_Throttle", axis),
        "IA_Brake": ensure_action("IA_Brake", axis),
        "IA_Steering": ensure_action("IA_Steering", axis),
        "IA_DRS": ensure_action("IA_DRS", boolean),
        "IA_ERS": ensure_action("IA_ERS", boolean),
        "IA_Camera": ensure_action("IA_Camera", boolean),
        "IA_Reset": ensure_action("IA_Reset", boolean),
        "IA_Pause": ensure_action("IA_Pause", boolean),
    }

    context_path = INPUT_DIRECTORY + "/IMC_ApexDrive"
    if not unreal.EditorAssetLibrary.does_asset_exist(context_path):
        factory = unreal.InputMappingContext_Factory()
        factory.set_editor_property("input_mapping_context_class", unreal.InputMappingContext)
        context = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "IMC_ApexDrive", INPUT_DIRECTORY, unreal.InputMappingContext, factory
        )
        unreal.EditorAssetLibrary.save_loaded_asset(context)

    unreal.log("APEX Phase 1: Enhanced Input actions and mapping context are present. The authoritative W/S/A/D/Shift/E/C/R bindings are built in AApexFormulaCar so the game can run without editor-only mapping mutations.")


main()
