"""Import revised mesh and baked PBR maps; retain existing skeletons and every clip."""
import unreal, pathlib, json
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
report=[]
for kind in ['Trex','Raptor','Trike']:
    folder=root/'Assets/Export/DinosaursModern'
    textures={}
    for channel in ['BaseColor','Roughness','Normal']:
        task=unreal.AssetImportTask();task.filename=str(folder/(kind+'_'+channel+'.png'));task.destination_path='/Game/Materials/Modern';task.automated=True;task.replace_existing=True;task.save=True
        A.import_asset_tasks([task]);tex=unreal.load_asset('/Game/Materials/Modern/'+kind+'_'+channel)
        tex.set_editor_property('srgb',channel=='BaseColor')
        tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_CHARACTER_NORMAL_MAP if channel=='Normal' else unreal.TextureGroup.TEXTUREGROUP_CHARACTER)
        if channel=='Normal':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
        elif channel=='Roughness':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_MASKS)
        E.save_loaded_asset(tex);textures[channel]=tex
    name='M_'+kind+'_Modern'
    mat=unreal.load_asset('/Game/Materials/'+name) or A.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
    M.delete_all_material_expressions(mat);mat.set_editor_property('used_with_skeletal_mesh',True)
    for j,(channel,prop) in enumerate([('BaseColor',unreal.MaterialProperty.MP_BASE_COLOR),('Roughness',unreal.MaterialProperty.MP_ROUGHNESS),('Normal',unreal.MaterialProperty.MP_NORMAL)]):
        n=M.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-450,j*210)
        n.texture=textures[channel]
        n.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if channel=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if channel=='Roughness' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
        M.connect_material_property(n,'R' if channel=='Roughness' else 'RGB',prop)
    spec=M.create_material_expression(mat,unreal.MaterialExpressionConstant,-150,600);spec.r=.38;M.connect_material_property(spec,'',unreal.MaterialProperty.MP_SPECULAR)
    M.recompile_material(mat);E.save_loaded_asset(mat)
    path='/Game/Dinosaurs/'+kind+'/'+kind
    mesh=unreal.load_asset(path);skeleton=mesh.skeleton
    before=[unreal.load_asset('/Game/Dinosaurs/'+kind+'/'+kind+'_'+clip).get_play_length() for clip in ['Idle','Walk','Run','Swim','Quick','Charge','Heavy','Jump','Brace','Death','Eat']]
    task=unreal.AssetImportTask();task.filename=str(folder/(kind+'.fbx'));task.destination_path='/Game/Dinosaurs/'+kind;task.destination_name=kind;task.automated=True;task.replace_existing=True;task.save=True;task.factory=unreal.FbxFactory()
    opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_mesh=True;opt.import_as_skeletal=True;opt.import_animations=False;opt.import_materials=False;opt.import_textures=False;opt.create_physics_asset=False;opt.skeleton=skeleton;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH
    opt.skeletal_mesh_import_data.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS
    opt.skeletal_mesh_import_data.set_editor_property('use_t0_as_ref_pose',False);task.options=opt;A.import_asset_tasks([task])
    mesh=unreal.load_asset(path)
    mats=list(mesh.materials)
    for slot in mats:slot.material_interface=mat
    mesh.materials=mats
    subsystem=unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    subsystem.regenerate_lod(mesh,3,True,False);E.save_loaded_asset(mesh)
    after=[unreal.load_asset('/Game/Dinosaurs/'+kind+'/'+kind+'_'+clip).get_play_length() for clip in ['Idle','Walk','Run','Swim','Quick','Charge','Heavy','Jump','Brace','Death','Eat']]
    assert before==after and mesh.skeleton==skeleton
    report.append(dict(species=kind,skeletonRetained=True,clipLengthsRetained=before,lods=subsystem.get_lod_count(mesh),materials=len(mats),vertices=[subsystem.get_num_verts(mesh,i) for i in range(subsystem.get_lod_count(mesh))]))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
(root/'Tests/Results/visual-modernization/dinosaur-import.json').write_text(json.dumps(report,indent=2))
unreal.log('MODERN_DINOSAURS_IMPORTED')
