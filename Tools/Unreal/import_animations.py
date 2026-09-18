"""Import only regenerated original clips, retaining existing meshes/materials."""
import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
tools=unreal.AssetToolsHelpers.get_asset_tools()
for kind in ['Trex','Raptor','Trike','Prey']:
    mesh=unreal.load_asset('/Game/Dinosaurs/'+kind+'/'+kind)
    for clip in ['Idle','Walk','Run','Swim','Quick','Charge','Heavy','Jump','Brace','Death','Eat']:
        task=unreal.AssetImportTask();task.filename=str(root/'Assets/Export/Dinosaurs'/(kind+'_'+clip+'.fbx'));task.destination_path='/Game/Dinosaurs/'+kind;task.destination_name=kind+'_'+clip
        task.automated=True;task.replace_existing=True;task.save=True;task.factory=unreal.FbxFactory()
        opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_mesh=False;opt.import_as_skeletal=True;opt.import_animations=True;opt.import_materials=False;opt.import_textures=False;opt.skeleton=mesh.get_editor_property('skeleton');opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION
        opt.anim_sequence_import_data.set_editor_property('animation_length',unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME);opt.anim_sequence_import_data.set_editor_property('use_default_sample_rate',True)
        task.options=opt;tools.import_asset_tasks([task])
        assert unreal.load_asset(task.destination_path+'/'+task.destination_name),task.destination_name
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
unreal.log('ANIMATION_IMPORT_SUCCESS')
