import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir()).resolve();A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
for name in ['SM_ConiferCanopy','SM_Grass']:
    task=unreal.AssetImportTask();task.filename=str(root/'Assets/Export/WorldModern'/(name+'.fbx'));task.destination_path='/Game/World';task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True;task.factory=unreal.FbxFactory()
    opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opt.import_mesh=True;opt.import_as_skeletal=False;opt.import_materials=False;opt.import_textures=False
    opt.static_mesh_import_data.set_editor_property('combine_meshes',True);opt.static_mesh_import_data.set_editor_property('auto_generate_collision',False);opt.static_mesh_import_data.set_editor_property('vertex_color_import_option',unreal.VertexColorImportOption.REPLACE)
    task.options=opt;A.import_asset_tasks([task]);mesh=unreal.load_asset('/Game/World/'+name)
    material=unreal.load_asset('/Game/Materials/M_NaturalFoliageSurface')
    slots=list(mesh.get_editor_property('static_materials'))
    for slot in slots:slot.set_editor_property('material_interface',material)
    mesh.set_editor_property('static_materials',slots);E.save_loaded_asset(mesh)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
