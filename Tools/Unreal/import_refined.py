"""Integrate the retained refined Blender meshes, baked original skin and reduced LODs."""
import unreal,pathlib,importlib.util,json
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
spec=importlib.util.spec_from_file_location('dino_import',root/'Tools/Unreal/import_dinosaurs.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
report=[]
for kind in ['Trex','Raptor','Trike','Prey']:
    folder=root/'Assets/Export/DinosaursRefined'
    task=unreal.AssetImportTask();task.filename=str(folder/(kind+'_SkinColor.png'));task.destination_path='/Game/Materials/Skins';task.automated=True;task.replace_existing=True;task.save=True;A.import_asset_tasks([task])
    texture=unreal.load_asset('/Game/Materials/Skins/'+kind+'_SkinColor');texture.set_editor_property('srgb',True);E.save_loaded_asset(texture)
    name='M_'+kind+'_BakedSkin';mat=unreal.load_asset('/Game/Materials/'+name)
    if not mat:
        mat=A.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew());mat.set_editor_property('used_with_skeletal_mesh',True)
        color=M.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-500,0);color.texture=texture;M.connect_material_property(color,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
        uv=M.create_material_expression(mat,unreal.MaterialExpressionTextureCoordinate,-700,200);uv.u_tiling=8;uv.v_tiling=8
        normal=M.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-450,200);normal.texture=unreal.load_asset('/Game/Materials/Textures/Scales_Normal');normal.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL;M.connect_material_expressions(uv,'',normal,'Coordinates');M.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL)
        rough=M.create_material_expression(mat,unreal.MaterialExpressionConstant,0,300);rough.r=.78;M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
        M.recompile_material(mat);E.save_loaded_asset(mat)
    mesh=base.import_species(kind,folder,mat)
    subsystem=unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    reduced=subsystem.regenerate_lod(mesh,3,False,False)
    E.save_loaded_asset(mesh)
    report.append(dict(species=kind,lods=subsystem.get_lod_count(mesh),reduced=bool(reduced),vertices=[subsystem.get_num_verts(mesh,i) for i in range(subsystem.get_lod_count(mesh))]))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
(root/'Tests/Results/refined-import.json').write_text(json.dumps(report,indent=2))
unreal.log('REFINED_IMPORT_SUCCESS')
