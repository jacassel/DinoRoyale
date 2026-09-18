"""Import original FBX meshes and clips; generate game materials and save all assets."""
import unreal, pathlib
ROOT=pathlib.Path(unreal.Paths.project_dir()).resolve()
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
LIB=unreal.EditorAssetLibrary
ML=unreal.MaterialEditingLibrary
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')

def mat(name,color,vertex=False,rough=.7):
    path='/Game/Materials/'+name
    m=unreal.load_asset(path)
    if m:
        m.set_editor_property('used_with_skeletal_mesh',True);ML.recompile_material(m);LIB.save_loaded_asset(m);return m
    m=TOOLS.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('used_with_skeletal_mesh',True)
    if vertex:
        c=ML.create_material_expression(m,unreal.MaterialExpressionVertexColor,-650,0)
        ML.connect_material_property(c,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
        uv=ML.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate,-650,220)
        uv.set_editor_property('u_tiling',28);uv.set_editor_property('v_tiling',28)
        noise=ML.create_material_expression(m,unreal.MaterialExpressionNoise,-350,220)
        noise.set_editor_property('scale',32);noise.set_editor_property('levels',2)
        # Subtle roughness variation gives scales readable specular breakup.
        r=ML.create_material_expression(m,unreal.MaterialExpressionConstant,-350,400);r.set_editor_property('r',rough)
        ML.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
    else:
        c=ML.create_material_expression(m,unreal.MaterialExpressionConstant3Vector,-300,0)
        c.set_editor_property('constant',unreal.LinearColor(*color,1));ML.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
        r=ML.create_material_expression(m,unreal.MaterialExpressionConstant,-300,150);r.set_editor_property('r',rough)
        ML.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
    ML.recompile_material(m);LIB.save_loaded_asset(m)
    return m

MATS={
 'Dino_Skin':mat('M_DinoSkin',(.3,.4,.2),True),
 'Dino_Horn':mat('M_Horn',(.62,.54,.37),rough=.50),
 'Dino_Claw':mat('M_Claw',(.095,.081,.052),rough=.46),
 'Dino_Mouth':mat('M_Mouth',(.075,.018,.022),rough=.54),
 'Dino_Eye':mat('M_Eye',(.95,.39,.025),rough=.22),
 'Dino_Pupil':mat('M_Pupil',(.004,.007,.003),rough=.2),
 'Dino_Tooth':mat('M_Tooth',(.82,.77,.60),rough=.43),
}

def imp(file,name,skeleton=None,anim=False):
    task=unreal.AssetImportTask();task.filename=str(file);task.destination_path='/Game/Dinosaurs/'+name.split('_')[0]
    task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
    task.factory=unreal.FbxFactory()
    opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False
    opt.import_mesh=not anim;opt.import_as_skeletal=True;opt.import_animations=anim
    opt.import_materials=False;opt.import_textures=False;opt.create_physics_asset=not anim
    opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION if anim else unreal.FBXImportType.FBXIT_SKELETAL_MESH
    if skeleton:opt.skeleton=skeleton
    opt.skeletal_mesh_import_data.set_editor_property('vertex_color_import_option',unreal.VertexColorImportOption.REPLACE)
    opt.skeletal_mesh_import_data.set_editor_property('normal_import_method',unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS)
    opt.skeletal_mesh_import_data.set_editor_property('use_t0_as_ref_pose',False)
    opt.anim_sequence_import_data.set_editor_property('animation_length',unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    opt.anim_sequence_import_data.set_editor_property('use_default_sample_rate',True)
    task.options=opt;TOOLS.import_asset_tasks([task]);return task.imported_object_paths

def import_species(kind,folder,skin_material=None):
    paths=imp(folder/(kind+'.fbx'),kind)
    mesh=unreal.load_asset('/Game/Dinosaurs/'+kind+'/'+kind)
    if not isinstance(mesh,unreal.SkeletalMesh):raise RuntimeError('No skeletal mesh: '+str(paths))
    mats=list(mesh.get_editor_property('materials'))
    for slot in mats:
        n=str(slot.get_editor_property('imported_material_slot_name'))
        if n in MATS:slot.set_editor_property('material_interface',skin_material if n=='Dino_Skin' and skin_material else MATS[n])
    mesh.set_editor_property('materials',mats)
    LIB.save_loaded_asset(mesh)
    skeleton=mesh.get_editor_property('skeleton')
    for clip in ['Idle','Walk','Run','Swim','Quick','Charge','Heavy','Jump','Brace','Death','Eat']:
        paths=imp(folder/(kind+'_'+clip+'.fbx'),kind+'_'+clip,skeleton,True)
        unreal.log('DINO_CLIP '+str(paths))
    unreal.log('DINO_MESH '+str(mesh.get_bounds()))
    return mesh

if __name__=='__main__':
    for kind in ['Trex','Raptor','Trike','Prey']:import_species(kind,ROOT/'Assets/Export/Dinosaurs')
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
    unreal.log('DINOSAUR_IMPORT_SUCCESS')
