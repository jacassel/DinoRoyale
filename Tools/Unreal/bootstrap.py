"""Create a saved launch map. Run via UnrealEditor-Cmd -run=pythonscript."""
import unreal

level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level.new_level('/Game/Maps/LostValley')
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log('DINOSAUR_BOOTSTRAP_SUCCESS')
