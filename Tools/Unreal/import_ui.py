import unreal,pathlib
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
root=pathlib.Path(unreal.Paths.project_dir())
t=unreal.AssetImportTask();t.filename=str(root/'Assets/Export/UI/T_ValleyMap.png');t.destination_path='/Game/UI';t.automated=True;t.replace_existing=True;t.save=True;A.import_asset_tasks([t])
tex=unreal.load_asset('/Game/UI/T_ValleyMap');tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI);E.save_loaded_asset(tex)
mat=unreal.load_asset('/Game/Materials/M_WorldSurface')
if not mat:mat=A.create_asset('M_WorldSurface','/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
def node(typ,**props):
    a=M.create_material_expression(mat,getattr(unreal,'MaterialExpression'+typ))
    for k,v in props.items():a.set_editor_property(k,v)
    return a
def link(a,b,p,out=''):M.connect_material_expressions(a,out,b,p)
c=node('VectorParameter',parameter_name='SurfaceTint',default_value=unreal.LinearColor(.20,.30,.10,1))
rand=node('PerInstanceRandom');mul=node('Multiply',const_b=.4);link(rand,mul,'A');add=node('Add',const_b=.8);link(mul,add,'A');base=node('Multiply');link(c,base,'A');link(add,base,'B');M.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
rough=node('Constant',r=.88);M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
mat.set_editor_property('two_sided',True);mat.set_editor_property('used_with_instanced_static_meshes',True);M.recompile_material(mat);E.save_loaded_asset(mat)
for meshname,color in [('SM_ConiferTrunk',(.16,.09,.042)),('SM_ConiferCanopy',(.065,.20,.085)),('SM_Fern',(.12,.27,.045)),('SM_Grass',(.22,.30,.07)),('SM_Boulder',(.26,.22,.17))]:
    name='MI_'+meshname[3:];mi=unreal.load_asset('/Game/Materials/'+name)
    if not mi:mi=A.create_asset(name,'/Game/Materials',unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
    M.set_material_instance_parent(mi,mat);M.set_material_instance_vector_parameter_value(mi,'SurfaceTint',unreal.LinearColor(*color,1));E.save_loaded_asset(mi)
    mesh=unreal.load_asset('/Game/World/'+meshname);mesh.set_material(0,mi);E.save_loaded_asset(mesh)
unreal.log('WORLD_MATERIAL_UI_IMPORT_SUCCESS')

blood=unreal.load_asset('/Game/Materials/M_Blood')
if not blood:
    blood=A.create_asset('M_Blood','/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
    c=M.create_material_expression(blood,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.38,.007,.003,1));M.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    r=M.create_material_expression(blood,unreal.MaterialExpressionConstant);r.set_editor_property('r',.36);M.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
    blood.set_editor_property('used_with_instanced_static_meshes',True);M.recompile_material(blood);E.save_loaded_asset(blood)
unreal.log('BLOOD_MATERIAL_READY')
