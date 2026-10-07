"""Import launch art, preserve other species, and record old/new material bindings."""
import unreal,pathlib,importlib.util,os,json
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
spec=importlib.util.spec_from_file_location('dino_import',root/'Tools/Unreal/import_dinosaurs.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
rows=[]
for kind in os.environ.get('DINO_IMPORT_SPECIES','Alberto,Anky,Pachy,Brachi').split(','):
    folder=root/'Assets/Export/Launch06'
    old=unreal.load_asset('/Game/Dinosaurs/'+kind+'/'+kind)
    before=[]
    if old:
        for slot in old.get_editor_property('materials'):
            mat=slot.get_editor_property('material_interface')
            before.append(dict(slot=str(slot.get_editor_property('imported_material_slot_name')),material=mat.get_path_name() if mat else None))
    textures={}
    for channel in ['SkinColor','SkinNormal']:
        task=unreal.AssetImportTask();task.filename=str(folder/(kind+'_'+channel+'.png'));task.destination_path='/Game/Materials/Launch06';task.automated=True;task.replace_existing=True;task.save=True;A.import_asset_tasks([task])
        tex=unreal.load_asset('/Game/Materials/Launch06/'+kind+'_'+channel);tex.srgb=channel=='SkinColor'
        if channel=='SkinNormal':tex.compression_settings=unreal.TextureCompressionSettings.TC_NORMALMAP;tex.lod_group=unreal.TextureGroup.TEXTUREGROUP_CHARACTER_NORMAL_MAP
        E.save_loaded_asset(tex);textures[channel]=tex
    name='M_'+kind+'_Launch06';mat=unreal.load_asset('/Game/Materials/Launch06/'+name) or A.create_asset(name,'/Game/Materials/Launch06',unreal.Material,unreal.MaterialFactoryNew())
    M.delete_all_material_expressions(mat);mat.set_editor_property('used_with_skeletal_mesh',True)
    for j,(channel,prop) in enumerate([('SkinColor',unreal.MaterialProperty.MP_BASE_COLOR),('SkinNormal',unreal.MaterialProperty.MP_NORMAL)]):
        n=M.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-450,j*220);n.texture=textures[channel];n.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if channel=='SkinNormal' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR;M.connect_material_property(n,'RGB',prop)
    rough=M.create_material_expression(mat,unreal.MaterialExpressionConstant,-100,500);rough.r=.80 if kind in ['Anky','Pachy'] else .73;M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS);M.recompile_material(mat);E.save_loaded_asset(mat)
    mesh=m.import_species(kind,folder,mat)
    mats=list(mesh.get_editor_property('materials'))
    # The color/normal atlases include the armor UV islands, formerly discarded by M_Horn.
    for slot in mats:
        if str(slot.get_editor_property('imported_material_slot_name'))=='Dino_Horn':slot.set_editor_property('material_interface',mat)
    mesh.set_editor_property('materials',mats)
    for clip in ['Quick2','Quick3','Hit','PivotLeft','PivotRight']:m.imp(folder/(kind+'_'+clip+'.fbx'),kind+'_'+clip,mesh.get_editor_property('skeleton'),True)
    subsystem=unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem);subsystem.regenerate_lod(mesh,3,True,False);E.save_loaded_asset(mesh)
    portrait=root/'Tests/Art/Launch06'/(kind+'_Idle.png')
    if portrait.exists():
        task=unreal.AssetImportTask();task.filename=str(portrait);task.destination_path='/Game/UI';task.destination_name='T_'+kind+'Portrait';task.automated=True;task.replace_existing=True;task.save=True;A.import_asset_tasks([task])
        tex=unreal.load_asset('/Game/UI/T_'+kind+'Portrait');tex.lod_group=unreal.TextureGroup.TEXTUREGROUP_UI;E.save_loaded_asset(tex)
    rows.append(dict(species=kind,before=before,after=[dict(slot=str(s.get_editor_property('imported_material_slot_name')),material=s.get_editor_property('material_interface').get_path_name()) for s in mats]))
    unreal.log('LAUNCH06_IMPORTED '+kind)
out=root/'Tests/Results/launch06/art';out.mkdir(parents=True,exist_ok=True)
(out/('materials-'+os.environ.get('DINO_IMPORT_SPECIES','all').replace(',','-')+'.json')).write_text(json.dumps(rows,indent=2))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
