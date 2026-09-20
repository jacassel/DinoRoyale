"""Import only the regenerated overview map; preserve all working materials."""
import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
t=unreal.AssetImportTask()
t.filename=str(root/'Assets/Export/UI/T_ValleyMap.png')
t.destination_path='/Game/UI';t.automated=True;t.replace_existing=True;t.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
tex=unreal.load_asset('/Game/UI/T_ValleyMap')
tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
unreal.EditorAssetLibrary.save_loaded_asset(tex)
unreal.log('COMPACT_VALLEY_MAP_IMPORTED')
