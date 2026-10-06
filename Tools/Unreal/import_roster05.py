"""Import only new roster assets, leaving all prior dinosaurs intact."""
import unreal,pathlib,importlib.util,os
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
spec=importlib.util.spec_from_file_location('dino_import',root/'Tools/Unreal/import_dinosaurs.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
for kind in ['Anky','Pachy','Brachi']:
    if os.environ.get('DINO_IMPORT_SPECIES') and kind!=os.environ['DINO_IMPORT_SPECIES']:continue
    folder=root/'Assets/Export/Roster05'
    if not (folder/(kind+'.fbx')).exists():continue
    mat=None
    if (folder/(kind+'_SkinColor.png')).exists():
        textures={}
        for channel in ['SkinColor','SkinNormal']:
            task=unreal.AssetImportTask();task.filename=str(folder/(kind+'_'+channel+'.png'));task.destination_path='/Game/Materials/Roster05';task.automated=True;task.replace_existing=True;task.save=True;A.import_asset_tasks([task])
            tex=unreal.load_asset('/Game/Materials/Roster05/'+kind+'_'+channel);tex.srgb=channel=='SkinColor'
            if channel=='SkinNormal':tex.compression_settings=unreal.TextureCompressionSettings.TC_NORMALMAP;tex.lod_group=unreal.TextureGroup.TEXTUREGROUP_CHARACTER_NORMAL_MAP
            E.save_loaded_asset(tex);textures[channel]=tex
        name='M_'+kind+'_Roster05';mat=unreal.load_asset('/Game/Materials/Roster05/'+name) or A.create_asset(name,'/Game/Materials/Roster05',unreal.Material,unreal.MaterialFactoryNew())
        M.delete_all_material_expressions(mat);mat.set_editor_property('used_with_skeletal_mesh',True)
        for j,(channel,prop) in enumerate([('SkinColor',unreal.MaterialProperty.MP_BASE_COLOR),('SkinNormal',unreal.MaterialProperty.MP_NORMAL)]):
            n=M.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-450,j*220);n.texture=textures[channel];n.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if channel=='SkinNormal' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR;M.connect_material_property(n,'RGB',prop)
        rough=M.create_material_expression(mat,unreal.MaterialExpressionConstant,-100,500);rough.r=.73;M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS);M.recompile_material(mat);E.save_loaded_asset(mat)
    mesh=m.import_species(kind,folder,mat)
    for clip in ['Quick2','Quick3','Hit','PivotLeft','PivotRight']:
        m.imp(folder/(kind+'_'+clip+'.fbx'),kind+'_'+clip,mesh.get_editor_property('skeleton'),True)
    unreal.log('ROSTER05_IMPORTED '+kind)
    subsystem=unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem);subsystem.regenerate_lod(mesh,3,True,False);E.save_loaded_asset(mesh)
    portrait=root/'Tests/Art/Roster05'/(kind+'_Idle.png')
    if portrait.exists():
        task=unreal.AssetImportTask();task.filename=str(portrait);task.destination_path='/Game/UI';task.destination_name='T_'+kind+'Portrait';task.automated=True;task.replace_existing=True;task.save=True;A.import_asset_tasks([task])
        tex=unreal.load_asset('/Game/UI/T_'+kind+'Portrait');tex.lod_group=unreal.TextureGroup.TEXTUREGROUP_UI;E.save_loaded_asset(tex)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
