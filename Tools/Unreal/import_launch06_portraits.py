import unreal,pathlib
r=pathlib.Path(unreal.Paths.project_dir()).resolve();a=unreal.AssetToolsHelpers.get_asset_tools()
for kind in ['Alberto','Anky','Pachy','Brachi']:
    task=unreal.AssetImportTask();task.filename=str(r/'Tests/Art/Launch06'/(kind+'_Idle.png'));task.destination_path='/Game/UI';task.destination_name='T_'+kind+'Portrait';task.automated=True;task.replace_existing=True;task.save=True;a.import_asset_tasks([task])
    tex=unreal.load_asset('/Game/UI/T_'+kind+'Portrait');tex.lod_group=unreal.TextureGroup.TEXTUREGROUP_UI;unreal.EditorAssetLibrary.save_loaded_asset(tex)
